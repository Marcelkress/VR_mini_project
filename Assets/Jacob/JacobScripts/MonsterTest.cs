using UnityEngine;
using UnityEngine.AI;
using System.Collections;

public class MonsterTest : MonoBehaviour, IDamagable
{
    [Header("Health & Combat")]
    public int health = 100;
    
    [Header("Horde AI Settings")]
    public Transform target; // Player target - assign this to make monster chase
    public float chaseSpeed = 3f;
    public float attackRange = 2.5f;
    public float attackCooldown = 2f;
    
    // Private variables
    private Animator animator;
    private NavMeshAgent agent;
    private float lastAttackTime = 0f;
    private bool isDead = false;
    private bool isAttacking = false;
    
    // Animation parameters
    private readonly int battleParam = Animator.StringToHash("battle");
    private readonly int movingParam = Animator.StringToHash("moving");

    void Start()
    {
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
        
        // Set up for chasing behavior
        animator.SetInteger(battleParam, 1); // Run mode for chasing
        agent.speed = chaseSpeed;
    }
    
    void Update()
    {
        if (isDead) return;
        
        // Simple horde AI: if target is assigned, chase and attack
        if (target != null)
        {
            float distanceToTarget = Vector3.Distance(transform.position, target.position);
            
            if (distanceToTarget <= attackRange)
            {
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
        if (Time.time - lastAttackTime < attackCooldown) return;

        StartCoroutine(LookAtTarget());
        // Trigger random attack animation
        isAttacking = true;
        lastAttackTime = Time.time;

        int attackType = Random.Range(0, 4);
        switch (attackType)
        {
            case 0:
                animator.SetInteger(movingParam, 4); // Attack 1
                break;
            case 1:
                animator.SetInteger(movingParam, 5); // Attack 2
                break;
            case 2:
                animator.SetInteger(movingParam, 7); // Bite
                break;
            case 3:
                animator.SetInteger(movingParam, 8); // Roar
                break;
        }

        // Reset animation after delay
        StartCoroutine(ResetAnimationAfterDelay(1.5f));
        StopCoroutine(LookAtTarget());
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

        if (velocity > 0.1f)
        {
            animator.SetInteger(movingParam, 2); // Run animation
        }
        else
        {
            animator.SetInteger(movingParam, 0); // Idle animation
        }
    }
    
    private IEnumerator ResetAnimationAfterDelay(float delay)
    {
        yield return new WaitForSeconds(delay);
        if (!isDead)
        {
            isAttacking = false;
            animator.SetInteger(movingParam, 0);
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

        TakeDamage(damager.damageAmount);
        Debug.Log($"Monster hit by {other.name} for {damager.damageAmount} damage.");
    }

    public void OnTriggerExit(Collider other)
    {
        // Optional: Handle logic when the collider exits, if needed
    }

    public void TakeDamage(int damage)
    {
        if (isDead) return;
        
        health -= damage;
        Debug.Log($"Monster took {damage} damage, remaining health: {health}");

        // Trigger hit animation
        int hitType = Random.Range(0, 2);
        if (hitType == 0)
        {
            animator.SetInteger(movingParam, 10); // Hit animation 1
        }
        else
        {
            animator.SetInteger(movingParam, 11); // Hit animation 2
        }
        
        // Reset hit animation after delay
        StartCoroutine(ResetAnimationAfterDelay(1f));

        if (health <= 0)
        {
            Die();
        }
    }

    private void Die()
    {
        if (isDead) return;
        
        isDead = true;
        Debug.Log("Monster died!");
        
        // Stop all movement
        agent.ResetPath();
        agent.enabled = false;
        
        // Trigger random death animation
        int deathType = Random.Range(0, 2);
        if (deathType == 0)
        {
            animator.SetInteger(movingParam, 12); // Death animation 1
        }
        else
        {
            animator.SetInteger(movingParam, 13); // Death animation 2
        }
        
        // Destroy after death animation
        StartCoroutine(DestroyAfterDelay(3f));
    }
    
    private IEnumerator DestroyAfterDelay(float delay)
    {
        yield return new WaitForSeconds(delay);
        Destroy(gameObject);
    }
}