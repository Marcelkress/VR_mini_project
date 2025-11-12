using UnityEngine;

[RequireComponent(typeof(Collider)), RequireComponent(typeof(Rigidbody))]
public class Damager : MonoBehaviour
{
    [Header("Impact Settings")]
    [Tooltip("The base damage dealt on any valid hit.")]
    public int baseDamage = 10;
    [Tooltip("How much damage is added per unit of velocity above the minimum.")]
    public float velocityDamageMultiplier = 2f;
    [Tooltip("How much knockback force is added per unit of velocity above the minimum.")]
    public float knockbackForceMultiplier = 5f;

    [Header("Velocity Thresholds")]
    [Tooltip("The minimum speed the weapon must travel to register a hit.")]
    public float minimumVelocity = 1f;
    [Tooltip("The maximum speed used for damage and knockback calculations.")]
    public float maxVelocity = 10f;
    
    [Header("Configuration")]
    public bool canKnockBack = true;
    [Tooltip("An empty GameObject on the controller/hand to accurately track swing speed.")]
    public Transform weaponVelocityTracker;

    private Vector3 lastTrackerPosition;
    private float currentSwingSpeed;

    private void Awake()
    {
        Rigidbody rb = GetComponent<Rigidbody>();
        rb.useGravity = false;

    }

    private void OnEnable()
    {
        // Reset position tracking when the object is enabled to prevent a large velocity spike
        if (weaponVelocityTracker != null)
        {
            lastTrackerPosition = weaponVelocityTracker.position;
        }
    }

    private void Update()
    {
        if (weaponVelocityTracker == null) return;
        
        // Calculate the speed of the tracker, which is more reliable than the sword's transform
        currentSwingSpeed = Vector3.Distance(weaponVelocityTracker.position, lastTrackerPosition) / Time.deltaTime;
        lastTrackerPosition = weaponVelocityTracker.position;
    }

    /// <summary>
    /// Calculates all hit-related data based on the current swing speed.
    /// </summary>
    /// <param name="damage">The calculated damage amount.</param>
    /// <param name="knockbackForce">The calculated knockback force.</param>
    /// <returns>True if the swing was fast enough to be a valid hit, false otherwise.</returns>
    public bool TryCalculateHit(out int damage, out float knockbackForce)
    {
        // Set default values for the 'out' parameters
        damage = 0;
        knockbackForce = 0f;

        // If we're not swinging fast enough, it's not a valid hit.
        if (currentSwingSpeed < minimumVelocity)
        {
            return false;
        }

        // Clamp the velocity to our defined min/max range for consistent results
        float effectiveVelocity = Mathf.Clamp(currentSwingSpeed, minimumVelocity, maxVelocity);

        // Calculate the "impact power" based on how much faster we are than the minimum
        float impactVelocity = effectiveVelocity - minimumVelocity;
        
        // 1. Calculate Damage
        damage = Mathf.RoundToInt(baseDamage + (impactVelocity * velocityDamageMultiplier));

        // 2. Calculate Knockback Force
        if (canKnockBack)
        {
            knockbackForce = impactVelocity * knockbackForceMultiplier;
        }

        return true;
    }

}