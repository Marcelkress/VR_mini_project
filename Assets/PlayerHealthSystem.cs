using System.Data;
using UnityEngine;

public class PlayerHealthSystem : MonoBehaviour
{
    public float maxHealth = 100f;
    public float currentHealth;

    void Start()
    {
        currentHealth = maxHealth;
    }

    void Update()
    {
        UpdateBloodvignette();
    }
    
    private void UpdateBloodvignette()
    {
        
        
        
    }

    public void TakeDamage(float damage)
    {
        currentHealth -= damage;
        Debug.Log("Player took " + damage + " damage. Current health: " + currentHealth);

        if (currentHealth <= 0)
        {
            Die();
        }
    }
    private void Die()
    {
        Debug.Log("Player has died.");
        // Add death logic here (e.g., respawn, game over screen, etc.)


    }

}
