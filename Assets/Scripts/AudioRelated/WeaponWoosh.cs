using UnityEngine;
using UnityEngine.XR;
using FMODUnity;
using FMOD.Studio;

public class VRWeaponWoosh : MonoBehaviour
{
    [Header("VR Settings")]
    [Tooltip("Which hand is holding this weapon?")]
    public XRNode hand = XRNode.RightHand;
   
    
    [Header("FMOD")]
    [Tooltip("FMOD event reference for the woosh sound.")]
    public EventReference wooshEvent;

    [Header("Swing Detection")]
    [Tooltip("Speed (m/s) required to trigger a woosh.")]
    public float velocityThreshold = 1.5f;

    [Tooltip("Minimum time between woosh sounds.")]
    public float cooldown = 0.25f;

    private InputDevice device;
    private float lastWooshTime;
    public bool IsInHand;

    [Header("FMOD")]
    [Tooltip("FMOD event reference for the Hit Sound.")]
    public EventReference BloddyHit;

    public GameObject HitSoundPosition;
    
    private void OnEnable()
    {
        InitDevice();
    }

    private void InitDevice()
    {
        device = InputDevices.GetDeviceAtXRNode(hand);
     
    }

    private void Update()
    {
        // Make sure we have a valid device
        if (!device.isValid)
        {
            InitDevice();
            return;
        }

        // Cooldown between wooshes
        if (Time.time - lastWooshTime < cooldown)
            return;

        // Get controller velocity
        if (device.TryGetFeatureValue(CommonUsages.deviceVelocity, out Vector3 velocity))
        {
            float speed = velocity.magnitude;

            // If we're swinging fast enough, play the woosh
            if (speed > velocityThreshold && IsInHand)  
            {
                PlayWoosh();
                lastWooshTime = Time.time;
            }
        }
    }

    private void PlayWoosh()
    {
        // Plays a 3D one-shot at this weapon's position
        RuntimeManager.PlayOneShotAttached(wooshEvent, gameObject);
    }
    
    public void IsWeaponInHand(bool inHand)
    {
        IsInHand = inHand;
    }

    public void PlayHitSound()
    {
        RuntimeManager.PlayOneShotAttached(BloddyHit, HitSoundPosition);
    }
    
    public void OnTriggerEnter(Collider monster)
    {
        if (monster.CompareTag("Monster"))
        {
            PlayHitSound();
        }
    }
}
