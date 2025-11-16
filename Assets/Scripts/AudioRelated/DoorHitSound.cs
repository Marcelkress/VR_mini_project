using UnityEngine;
using FMODUnity;

public class DoorHitSound : MonoBehaviour
{
    [Header("FMOD")]
    public StudioEventEmitter hitEmitter;   // impact on door
    public StudioEventEmitter openEmitter;  // door creak/open sound

    // Called from WeaponTip when it detects a hit on this door
    public void OnHit(RaycastHit hit)
    {
        if (hitEmitter != null)
            hitEmitter.Play();   // play impact every time

        if (openEmitter != null)
            openEmitter.Play();  // play creak EVERY time
    }
}