using UnityEngine;
using System.Collections;

[RequireComponent(typeof(Collider)), RequireComponent(typeof(Rigidbody))]
public class Damager : MonoBehaviour
{
    [Header("Damage Settings")]
    public int baseDamageAmount = 10;
    public float velocityMultiplier = 2f;
    public float minimumVelocity = 1f; // Minimum velocity required to cause damage
    public float maxVelocity = 10f; // Cap the velocity for damage calculation

    public bool canKnockBack = true;
    public float knockBackForceMultiplier = 1.5f; // Multiplier for knockback force calculation

    public Transform weaponVelocityTracker;
    
    [Header("Debug")]
    public bool showDebugInfo = true;
    
    private Rigidbody rb;
    private Vector3 lastPosition;
    private float currentVelocity;
    
    void Start()
    {
        GetComponent<Collider>().isTrigger = false;
        rb = GetComponent<Rigidbody>();
        rb.useGravity = false;
        
        lastPosition = transform.position;
    }
    
    void Update()
    {
        if (weaponVelocityTracker == null)
            return;
        
        // Calculate velocity manually for more accurate VR tracking
        currentVelocity = UnityEngine.Vector3.Distance(weaponVelocityTracker.position, lastPosition) / Time.deltaTime;
        lastPosition = transform.position;
        
        if (showDebugInfo && currentVelocity > minimumVelocity)
        {
          //  Debug.Log($"Weapon velocity: {currentVelocity:F2} m/s");
        }
    }

    // Helper: compute the raw (unclamped) final velocity combining tracked and rigidbody velocity
    private float GetRawFinalVelocity()
    {
        float rbVelocity = rb.linearVelocity.magnitude;
        return Mathf.Max(currentVelocity, rbVelocity);
    }

    public int GetDamageAmount()
    {
        float finalVelocity = GetRawFinalVelocity();

        // Only cause damage if moving fast enough
        if (finalVelocity < minimumVelocity)
        {
            return 0;
        }

        // Cap the velocity and calculate damage
        finalVelocity = Mathf.Clamp(finalVelocity, minimumVelocity, maxVelocity);
        float velocityDamage = (finalVelocity - minimumVelocity) * velocityMultiplier;
        int totalDamage = Mathf.RoundToInt(baseDamageAmount + velocityDamage);

        if (showDebugInfo)
        {
            Debug.Log($"Velocity: {finalVelocity:F2}, Base Damage: {baseDamageAmount}, Velocity Damage: {velocityDamage:F2}, Total: {totalDamage}");
        }

        return totalDamage;
    }
    
    public float GetKnockBackForce()
    {
        if (!canKnockBack) return 0f;
        // Knockback force proportional to velocity
        float finalVelocity = GetRawFinalVelocity();

        if (finalVelocity < minimumVelocity)
        {
            return 0f;
        }

        finalVelocity = Mathf.Clamp(finalVelocity, minimumVelocity, maxVelocity);
        float knockBackForce = (finalVelocity - minimumVelocity) * knockBackForceMultiplier; // Adjust multiplier as needed

        return knockBackForce;
    }
}
