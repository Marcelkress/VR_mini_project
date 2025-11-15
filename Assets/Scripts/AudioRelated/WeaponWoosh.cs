using UnityEngine;
using UnityEngine.XR;
using FMODUnity;
using FMOD.Studio;

public class VRWeaponWoosh : MonoBehaviour
{
    [Header("VR Settings")]
    [Tooltip("Which hand is currently holding this weapon?")]
    public XRNode Hand = XRNode.RightHand;   // default, will be overridden at grab

    [Header("FMOD - Woosh")]
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

    [Header("FMOD - Hit")]
    [Tooltip("FMOD event reference for the Hit Sound.")]
    public EventReference BloddyHit;

    public GameObject HitSoundPosition;

    private void OnEnable()
    {
        InitDevice();
    }

    private void InitDevice()
    {
        device = InputDevices.GetDeviceAtXRNode(Hand);
    }

    private void Update()
    {
        // Make sure we have a valid device for the current hand
        if (!device.isValid)
        {
            InitDevice();
            if (!device.isValid) return;
        }

        // Weapon must be in hand and not on cooldown
        if (!IsInHand || Time.time - lastWooshTime < cooldown)
            return;

        // Get controller velocity
        if (device.TryGetFeatureValue(CommonUsages.deviceVelocity, out Vector3 velocity))
        {
            float speed = velocity.magnitude;

            if (speed > velocityThreshold)
            {
                PlayWoosh();
                lastWooshTime = Time.time;
            }
        }
    }

    private void PlayWoosh()
    {
        RuntimeManager.PlayOneShotAttached(wooshEvent, gameObject);
    }

    public void IsWeaponInHand(bool inHand)
    {
        IsInHand = inHand;
    }

    public void SetHand(XRNode hand)
    {
        Hand = hand;
        InitDevice();
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
