using System.Data;
using UnityEngine;
using UnityEngine.Events;

public class PlayerHealthSystem : MonoBehaviour
{
    public int maxHealth = 100;
    public int currentHealth;
    private bool lowHealthTriggered = false;

    public UnityEvent LowHealthEvent, TakeDamageEvent, DeathEvent;

    void Start()
    {
        currentHealth = maxHealth;
    }

    void Update()
    {
        DynamicEvents();
    }

    private void DynamicEvents()
    {
        // Check if health is low (below 30%) and trigger event once
        if (currentHealth <= maxHealth * 0.3f && !lowHealthTriggered)
        {
            LowHealthEvent.Invoke();
            lowHealthTriggered = true;
        }

        // Reset flag when health is restored above low threshold
        if (currentHealth > maxHealth * 0.3f && lowHealthTriggered)
        {
            lowHealthTriggered = false;
        }
    }
    
    public void Heal(int healAmount)
    {
        if (healAmount <= 0 || currentHealth == maxHealth) return;

        currentHealth += healAmount;
        if (currentHealth > maxHealth)
        {
            currentHealth = maxHealth;
        }
    }

    public void TakeDamage(int damage)
    {
        currentHealth -= damage;
        Debug.Log("Player took " + damage + " damage. Current health: " + currentHealth);
        TakeDamageEvent.Invoke();
        if (currentHealth <= 0)
        {
            Die();
        }
    }
    private void Die()
    {
        Debug.Log("Player has died.");
        DeathEvent.Invoke();

    }

}
