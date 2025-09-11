using UnityEngine;

[RequireComponent(typeof(Animator)), RequireComponent(typeof(Collider)), RequireComponent(typeof(Rigidbody))]
public class MonsterTest : MonoBehaviour, IDamagable
{
    public int health = 100;

    private Animator animator;

    void Start()
    {

        animator = GetComponent<Animator>();
        if (animator == null)
        {
            Debug.LogError("Animator component not found on the monster.");
        }
    }

    public void OnTriggerEnter(Collider other)
    {
        var damager = other.GetComponent<Damager>();
        if (damager == null) return;

        TakeDamage(damager.damageAmount);
        animator.SetTrigger("Hit");

    }

    public void OnTriggerExit(Collider other)
    {
        animator.ResetTrigger("Hit");
        // Optional: Handle logic when the collider exits, if needed
    }

    public void TakeDamage(int damage)
    {
        health -= damage;
        Debug.Log($"Monster took {damage} damage, remaining health: {health}");

        if (health <= 0)
        {
            Die();
        }
    }

    private void Die()
    {
        Debug.Log("Monster died!");
        Destroy(gameObject);
    }
}