using UnityEngine;

[RequireComponent(typeof(Collider)), RequireComponent(typeof(Rigidbody))]
public class Damager : MonoBehaviour
{
    [Header("Damage Settings")]
    public int baseDamageAmount = 10;
    public float velocityMultiplier = 2f;
    public float minimumVelocity = 1f; // Minimum velocity required to cause damage
    public float maxVelocity = 10f; // Cap the velocity for damage calculation
    
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
        // Calculate velocity manually for more accurate VR tracking
        currentVelocity = Vector3.Distance(transform.position, lastPosition) / Time.deltaTime;
        lastPosition = transform.position;
        
        if (showDebugInfo && currentVelocity > minimumVelocity)
        {
          //  Debug.Log($"Weapon velocity: {currentVelocity:F2} m/s");
        }
    }
    
    public int GetDamageAmount()
    {
        // Use the higher of rigidbody velocity or calculated velocity
        float rbVelocity = rb.linearVelocity.magnitude;
        float finalVelocity = Mathf.Max(currentVelocity, rbVelocity);
        
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
}
