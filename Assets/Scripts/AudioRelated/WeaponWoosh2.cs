using UnityEngine;
using UnityEngine.XR;
using FMODUnity;
using FMOD.Studio;

public class VRWeaponWoosh2 : MonoBehaviour
{
    [Header("VR Settings")]
    [Tooltip("Which hand is holding this weapon?")]
    public XRNode Righthand = XRNode.RightHand;
    public XRNode Lefthand = XRNode.LeftHand;

    // The hand currently holding the weapon
    private XRNode currentHand;

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
    private bool IsInHand;

    [Header("FMOD")]
    [Tooltip("FMOD event reference for the Hit Sound.")]
    public EventReference BloddyHit;

    public GameObject HitSoundPosition;

    private void Awake()
    {
        // Default to right hand; will be overridden by SetHand when grabbed
        currentHand = Righthand;
    }

    private void OnEnable()
    {
        InitDevice();
    }

    private void InitDevice()
    {
        device = InputDevices.GetDeviceAtXRNode(currentHand);
    }

    private void Update()
    {
        // Only care if weapon is actually held
        if (!IsInHand)
            return;

        // Make sure we have a valid device
        if (!device.isValid)
        {
            InitDevice();
            if (!device.isValid)
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
            if (speed > velocityThreshold)
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

        // When we pick it up again, refresh the device
        if (inHand)
            InitDevice();
    }

    // Called by grab logic to select left/right controller
    public void SetHand(XRNode hand)
    {
        currentHand = hand;
        InitDevice();
    }

    public void PlayHitSound()
    {
        RuntimeManager.PlayOneShotAttached(BloddyHit, HitSoundPosition);
    }

    public void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Monster"))
        {
            PlayHitSound();
        }
    }
}
