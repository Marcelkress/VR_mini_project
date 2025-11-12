using UnityEngine;
using UnityEngine.AI;
using System.Collections;
using UnityEngine.Events;

[RequireComponent(typeof(Animator), typeof(NavMeshAgent), typeof(Collider))]
public class MonsterTest : MonoBehaviour, IDamagable
{
    [Header("Health & Combat")]
    [SerializeField] private MonsterData monsterData;
    [SerializeField] private GameObject bloodEffectPrefab;

    [Header("AI Settings")]
    public Transform target; // Player target
    [SerializeField] private bool startCrawling = false;
    [SerializeField] private float crawlSpeed = 1f;

    [Header("Knockback Settings")]
    [Tooltip("An animation curve to control the knockback speed over time. (0,1) is start, (1,0) is end.")]
    [SerializeField] private AnimationCurve knockbackCurve = AnimationCurve.EaseInOut(0, 1, 1, 0);
    [Tooltip("How long the knockback effect should last.")]
    [SerializeField] private float knockbackDuration = 0.4f;

    // --- Component References ---
    private Animator animator;
    private NavMeshAgent agent;

    // --- State Management ---
    private bool isDead = false;
    private bool isAttacking = false;
    private float lastAttackTime = 0f;
    private Coroutine knockbackCoroutine; // To manage the knockback state

    // --- Animation Hashes (for performance) ---
    private readonly int isCrawlingParam = Animator.StringToHash("IsCrawling");
    private readonly int runningParam = Animator.StringToHash("Running");
    private readonly int attackParam = Animator.StringToHash("Attack");
    private readonly int takeHitParam = Animator.StringToHash("TakeHit");

    // --- Events ---
    public UnityEvent AttackEvent, TakeDamageEvent, DeadEvent;

    #region Unity Methods

    private void Awake()
    {
        monsterData.Initialize();
        animator = GetComponent<Animator>();
        agent = GetComponent<NavMeshAgent>();
    }

    private void Start()
    {
        animator.SetBool(isCrawlingParam, startCrawling);
        SetCrawling(startCrawling); // Use the method to set speed correctly
    }

    private void Update()
    {
        // A monster that is dead or being knocked back should not run AI logic.
        if (isDead || knockbackCoroutine != null)
        {
            return;
        }

        if (target != null)
        {
            float distanceToTarget = Vector3.Distance(transform.position, target.position);
            
            if (distanceToTarget <= monsterData.attackRange)
            {
                agent.ResetPath();
                AttackTarget();
            }
            else
            {
                isAttacking = false; // Ensure we are not stuck in attacking state if player moves away
                agent.isStopped = false;
                agent.destination = target.position;
            }
        }
        UpdateMovementAnimation();
    }

    private void OnTriggerEnter(Collider other)
    {
        if (isDead) return;

        if (other.TryGetComponent<Damager>(out Damager damager))
        {
            if (damager.TryCalculateHit(out int damage, out float knockbackForce))
            {
                // 1. Process Damage
                TakeDamage(damage);

                // 2. Play Effects
                animator.SetTrigger(takeHitParam);
                if (bloodEffectPrefab != null)
                {
                    Instantiate(bloodEffectPrefab, other.ClosestPoint(transform.position), Quaternion.identity);
                }
                
                // 3. Apply Knockback
                if (knockbackForce > 0)
                {
                    Vector3 hitSourcePosition = other.transform.position;
                    ApplyKnockback(knockbackForce, hitSourcePosition);
                }
            }
        }
    }

    #endregion

    #region Combat & Damage

    public void TakeDamage(int damage)
    {
        if (isDead) return;
        
        monsterData.currentHealth -= damage;
        TakeDamageEvent.Invoke();
        
        Debug.Log($"Monster took {damage} damage, current health: {monsterData.currentHealth}/{monsterData.maxHealth}");
        
        if (monsterData.currentHealth <= 0)
        {
            Die();
        }
    }

    public void Die()
    {
        if (isDead) return;
        
        isDead = true;
        DeadEvent.Invoke();
        Debug.Log("Monster died!");

        // Stop all movement and AI
        if (agent.isOnNavMesh) agent.enabled = false;
        GetComponent<Collider>().enabled = false;
        
        // Stop the animator to allow for ragdoll physics or a static death pose
        animator.enabled = false;
    }

    private void AttackTarget()
    {
        if (isAttacking || Time.time - lastAttackTime < monsterData.attackCooldown) return;

        isAttacking = true;
        lastAttackTime = Time.time;

        agent.isStopped = true;
        agent.velocity = Vector3.zero;

        // Look at target during attack
        Vector3 lookDirection = (target.position - transform.position);
        lookDirection.y = 0;
        transform.rotation = Quaternion.LookRotation(lookDirection);

        animator.SetTrigger(attackParam);
        AttackEvent.Invoke();
        GetComponent<FMODUnity.StudioEventEmitter>()?.Play();

        // Perform damage check after a short delay to sync with animation
        StartCoroutine(PerformAttackDamage(0.1f)); // Magic Number oh no, should be synced with animation, we can improve later with animation events
        StartCoroutine(ResetAttackingState(monsterData.attackCooldown));
    }

    // Separated the damage logic for better timing with animations
    private IEnumerator PerformAttackDamage(float delay)
    {
        yield return new WaitForSeconds(delay);

        if (isDead) yield break;
        
        // Check if target is still in range after the animation windup
        if (Vector3.Distance(transform.position, target.position) <= monsterData.attackRange + 0.5f) // Small buffer
        {
            if (target.TryGetComponent<PlayerHealthSystem>(out var playerHealth))
            {
                playerHealth.TakeDamage(monsterData.damageAmount);
            }
        }
    }
    
    private IEnumerator ResetAttackingState(float delay)
    {
        yield return new WaitForSeconds(delay);
        isAttacking = false;
        if (!isDead && knockbackCoroutine == null)
        {
            agent.isStopped = false;
        }
    }

    #endregion

    #region Knockback System

    // NEW & IMPROVED: This is the public method to call.
    public void ApplyKnockback(float force, Vector3 hitSourcePosition)
    {
        if (isDead || force <= 0f) return;

        // If a knockback is already happening, stop it and start the new one.
        // This makes the monster react realistically to rapid hits.
        if (knockbackCoroutine != null)
        {
            StopCoroutine(knockbackCoroutine);
        }

        Vector3 knockbackDirection = (transform.position - hitSourcePosition).normalized;
        knockbackDirection.y = 0; // Keep knockback horizontal

        // Failsafe if direction is somehow zero
        if (knockbackDirection.sqrMagnitude < 0.001f)
        {
            knockbackDirection = -transform.forward;
        }

        knockbackCoroutine = StartCoroutine(KnockbackCoroutine(knockbackDirection, force));
    }
    
    private IEnumerator KnockbackCoroutine(Vector3 direction, float initialForce)
    {
        // --- On Knockback Start ---
        isAttacking = false; // Interrupt any attack
        agent.isStopped = true;
        agent.ResetPath();

        float elapsedTime = 0f;
        while (elapsedTime < knockbackDuration)
        {
            // The curve evaluates from 1 down to 0, creating a smooth deceleration.
            float speedMultiplier = knockbackCurve.Evaluate(elapsedTime / knockbackDuration);
            float currentSpeed = initialForce * speedMultiplier;

            agent.Move(direction * currentSpeed * Time.deltaTime);

            elapsedTime += Time.deltaTime;
            yield return null;
        }

        // --- On Knockback End ---
        if (!isDead)
        {
            agent.isStopped = false;
        }
        knockbackCoroutine = null; // Signal that the knockback is finished.
    }

    #endregion

    #region Animation & Movement

    private void UpdateMovementAnimation()
    {
        float velocity = agent.velocity.magnitude;
        int runState = (velocity > 0.1f) ? (animator.GetBool(isCrawlingParam) ? 1 : 2) : 0;
        animator.SetInteger(runningParam, runState);
    }

    public void SetCrawling(bool crawling)
    {
        startCrawling = crawling;
        animator.SetBool(isCrawlingParam, crawling);
        agent.speed = crawling ? crawlSpeed : monsterData.chaseSpeed;
    }

    public void SetTarget(Transform newTarget)
    {
        target = newTarget;
    }

    #endregion
}