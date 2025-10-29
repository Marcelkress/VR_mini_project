using UnityEngine;
using UnityEngine.AI;
using System.Collections;
using NUnit.Compatibility;
using UnityEngine.Events;

public class MonsterTest : MonoBehaviour, IDamagable
{
    [Header("Health & Combat")]
    [SerializeField] private MonsterData monsterData;

    [Header("Horde AI Settings")]
    public Transform target; // Player target - assign this to make monster chase
    [SerializeField]
    [Tooltip("If true, the monster will use its crawling animation and crawl speed.")]
    private bool startCrawling = false;

    // Private variables
    private Animator animator;
    private NavMeshAgent agent;
    private float lastAttackTime = 0f;
    private bool isDead = false;
    private bool isAttacking = false;
    
    // Animation parameters matching the animator controller
    private readonly int isCrawlingParam = Animator.StringToHash("IsCrawling");
    private readonly int runningParam = Animator.StringToHash("Running");
    private readonly int attackParam = Animator.StringToHash("Attack");
    private readonly int deathParam = Animator.StringToHash("Death");
    
    // Crawl speed (optional override). If zero, uses monsterData.speed
    [SerializeField]
    private float crawlSpeed = 1f;
    
    public UnityEvent AttackEvent, TakeDamageEvent, DeadEvent, ScreamEvent;

    void Start()
    {
        monsterData.Initialize();
        
        animator = GetComponent<Animator>();
        agent = GetComponent<NavMeshAgent>();
        
        if (animator == null)
        {
            Debug.LogError("Animator component not found on the monster.");
        }
        
        if (agent == null)
        {
            Debug.LogError("NavMeshAgent component not found on the monster.");
        }
        
        // Initialize animation state - start in idle
        animator.SetBool(isCrawlingParam, startCrawling);
        animator.SetInteger(runningParam, 0); // 0 = Idle
        animator.SetInteger(attackParam, 0);
        
    // Set up NavMeshAgent for chasing (respect crawling)
    agent.speed = startCrawling ? (crawlSpeed > 0f ? crawlSpeed : monsterData.speed) : monsterData.chaseSpeed; // chase speed if not crawling
    }
    
    void Update()
    {
        if (isDead) return;
        
        // Simple horde AI: if target is assigned, chase and attack
        if (target != null)
        {
            float distanceToTarget = Vector3.Distance(transform.position, target.position);
            
            if (distanceToTarget <= monsterData.attackRange)
            {
                agent.stoppingDistance = monsterData.attackRange - 0.5f;
                // Stop and attack
                agent.ResetPath();
                AttackTarget();
            }
            else
            {
                // Chase the target
                agent.destination = target.position;
                isAttacking = false;
            }
            
            // Update movement animation based on velocity
            UpdateMovementAnimation();
        }
    }

    private void AttackTarget()
    {
        if (isAttacking) return;

        // Check attack cooldown
        if (Time.time - lastAttackTime < monsterData.attackCooldown) return;

        // Trigger attack animation using trigger parameter
        isAttacking = true;
        AttackEvent.Invoke();
        lastAttackTime = Time.time;

        // stop movement during attack
        agent.ResetPath();
        agent.velocity = Vector3.zero;
        agent.isStopped = true;


        animator.SetTrigger(attackParam);

        // Look at target during attack
        StartCoroutine(LookAtTarget());

        OverlapSphereDamage();

        // Reset attacking flag after animation
        StartCoroutine(ResetAttackingState(monsterData.attackCooldown));
        agent.isStopped = false; // Resume movement after attack
    }

    private void OverlapSphereDamage()
    {
        Collider[] hitColliders = Physics.OverlapSphere(transform.position, monsterData.attackRange);
        foreach (var hitCollider in hitColliders)
        {
            if (hitCollider.transform == target)
            {
                PlayerHealthSystem playerHealthSystem = hitCollider.GetComponent<PlayerHealthSystem>();
                if (playerHealthSystem != null)
                {
                    playerHealthSystem.TakeDamage(monsterData.damageAmount);
                    Debug.Log("Monster dealt " + monsterData.damageAmount + " damage to " + hitCollider.name);

                    
                }
            }
        }
    }
    
   private IEnumerator LookAtTarget()
    {
        while (isAttacking)
        {
            Vector3 lookDirection = (target.position - transform.position).normalized;
            lookDirection.y = 0;
            if (lookDirection != Vector3.zero)
            {
                transform.rotation = Quaternion.Slerp(transform.rotation, Quaternion.LookRotation(lookDirection), Time.deltaTime * 5f);
            }
            yield return null;
        }
    }
    
    private void UpdateMovementAnimation()
    {
        if (isAttacking) return;

        float velocity = agent.velocity.magnitude;

        // If crawling, force the crawling bool and don't set running states
        if (animator.GetBool(isCrawlingParam))
        {
            // Ensure agent speed respects crawling
            agent.speed = crawlSpeed > 0f ? crawlSpeed : monsterData.speed;
            
            // Set Running to 1 for crawl movement, 0 for crawl idle
            if (velocity > 0.1f)
            {
                animator.SetInteger(runningParam, 1); // Crawl moving
            }
            else
            {
                animator.SetInteger(runningParam, 0); // Crawl idle
            }
            return;
        }

        // Normal running/idle states
        if (velocity > 0.1f)
        {
            animator.SetInteger(runningParam, 2); // Run animation
        }
        else
        {
            animator.SetInteger(runningParam, 0); // Idle animation
        }
    }

    // Publicly switch crawling mode at runtime
    public void SetCrawling(bool crawling)
    {
        animator.SetBool(isCrawlingParam, crawling);
        if (crawling)
        {
            agent.speed = crawlSpeed > 0f ? crawlSpeed : monsterData.speed;
        }
        else
        {
            agent.speed = monsterData.chaseSpeed;
        }
    }
    
    private IEnumerator ResetAnimationAfterDelay(float delay)
    {
        yield return new WaitForSeconds(delay);
        if (!isDead)
        {
            isAttacking = false;
        }
    }
    
    private IEnumerator ResetAttackingState(float delay)
    {
        yield return new WaitForSeconds(delay);
        if (!isDead)
        {
            isAttacking = false;
        }
    }
    
    // Public method to set target (for easy horde management)
    public void SetTarget(Transform newTarget)
    {
        target = newTarget;
    }
    
    // Public method to force attack (for testing/debugging)
    public void ForceAttack()
    {
        if (!isDead)
        {
            AttackTarget();
        }
    }
    
    // Check if monster is currently attacking
    public bool IsAttacking()
    {
        return isAttacking;
    }

    public void OnTriggerEnter(Collider other)
    {
        var damager = other.GetComponent<Damager>();
        if (damager == null) return;

        int damage = damager.GetDamageAmount();

        // Only take damage if the weapon is moving fast enough
        if (damage > 0)
        {
            animator.SetTrigger("TakeHit");
            TakeDamage(damage);
            Debug.Log($"Monster hit by {other.name} for {damage} damage based on velocity.");

            ApplyKnockBack(damager.GetKnockBackForce(), other.transform.position);
        }
        else
        {
            Debug.Log($"Weapon {other.name} hit but was moving too slowly to cause damage.");
        }
    }
    
    private void ApplyKnockBack(float force, UnityEngine.Vector3 currentHitPosition)
    {
        if (force <= 0f) return;

        Vector3 knockBackDirection = (transform.position - currentHitPosition).normalized;
        knockBackDirection.y = 0; // Keep knockback horizontal

        // Apply knockback by moving the NavMeshAgent backwards
        Vector3 knockBackTarget = transform.position + knockBackDirection * force;
        agent.Warp(knockBackTarget);
        
        Debug.Log($"Monster knocked back by force {force}.");
    }

    public void OnTriggerExit(Collider other)
    {
        // Optional: Handle logic when the collider exits, if needed
    }

    public void TakeDamage(int damage)
    {
        if (isDead) return;
        
        monsterData.currentHealth -= damage;
        
        TakeDamageEvent.Invoke();

        // Trigger hit animation - you can add hit reactions to your animator if needed
        // For now, we'll just log the damage
        
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

        // Stop all movement
        agent.ResetPath();
        agent.enabled = false;

        GetComponent<Collider>().enabled = false; // Disable collider to prevent further interactions
    
        // Disable animator to enable ragdoll        
        animator.enabled = false; // Disable animator to stop all animations
        
        
        
    }
    
}