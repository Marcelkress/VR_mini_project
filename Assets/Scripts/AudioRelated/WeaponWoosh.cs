using UnityEngine;
using FMODUnity;
using FMOD.Studio;

public class VRWeaponWoosh : MonoBehaviour
{
    [Header("Swing Detection")]
    [Tooltip("Speed (m/s) required to trigger a woosh.")]
    public float velocityThreshold = 1.5f;

    [Tooltip("Minimum time between woosh sounds.")]
    public float cooldown = 0.25f;

    [Header("FMOD - Woosh")]
    [Tooltip("FMOD event reference for the woosh sound.")]
    public EventReference wooshEvent;

    [Header("FMOD - Hit")]
    [Tooltip("FMOD event reference for the Hit Sound.")]
    public EventReference BloddyHit;

    public GameObject HitSoundPosition;

    private float lastWooshTime;
    private bool IsInHand;

    private Vector3 lastPosition;
    private bool hasLastPosition;

    private void OnEnable()
    {
        hasLastPosition = false;
    }

    private void Update()
    {
        // Need at least one frame to initialize position
        if (!hasLastPosition)
        {
            lastPosition = transform.position;
            hasLastPosition = true;
            return;
        }

        // Weapon must be in hand and not on cooldown
        if (!IsInHand || Time.time - lastWooshTime < cooldown)
        {
            lastPosition = transform.position;
            return;
        }

        // Approximate velocity from movement of the weapon in world space
        Vector3 displacement = transform.position - lastPosition;
        float speed = displacement.magnitude / Time.deltaTime;

        if (speed > velocityThreshold)
        {
            PlayWoosh();
            lastWooshTime = Time.time;
        }

        lastPosition = transform.position;
    }

    private void PlayWoosh()
    {
        RuntimeManager.PlayOneShotAttached(wooshEvent, gameObject);
    }

    /// <summary>
    /// Call this when the weapon is picked up / dropped.
    /// </summary>
    public void IsWeaponInHand(bool inHand)
    {
        IsInHand = inHand;

        // Reset last position when picked up to avoid instant woosh
        if (inHand)
        {
            hasLastPosition = false;
        }
    }

    public void PlayHitSound()
    {
        RuntimeManager.PlayOneShotAttached(BloddyHit, HitSoundPosition);
    }

    private void OnTriggerEnter(Collider monster)
    {
        if (monster.CompareTag("Monster"))
        {
            PlayHitSound();
        }
    }
}
