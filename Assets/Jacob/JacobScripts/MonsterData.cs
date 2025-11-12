using Unity.VisualScripting;
using UnityEngine;

[CreateAssetMenu(fileName = "Health", menuName = "Health/health")]
public class MonsterData : ScriptableObject
{
    
    [Header("Health Settings")]
    public int currentHealth;
    public int maxHealth = 100;

    [Header("Movement & Combat")]
    public float speed = 1f;
    public float chaseSpeed = 3f;
    public float attackRange = 2.5f;
    public float attackCooldown = 2f;
    public float hitReactionTime = 0.5f;
    public int damageAmount = 10;

    public AnimationCurve knockBackCurve = AnimationCurve.EaseInOut(0,1,1,0);


    public void Initialize()
    {
        currentHealth = maxHealth;

        if (maxHealth <= 0)
        {
            Debug.LogWarning("Max health is set to 0 or less. Setting to default value of 100.");
            maxHealth = 100;
        }


    }
}
