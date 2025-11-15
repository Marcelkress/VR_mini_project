using UnityEngine;
using UnityEngine.XR.Interaction.Toolkit;

[RequireComponent(typeof(UnityEngine.XR.Interaction.Toolkit.Interactables.XRGrabInteractable))]
public class WeaponGrabHandler : MonoBehaviour
{
    private UnityEngine.XR.Interaction.Toolkit.Interactables.XRGrabInteractable grab;
    private VRWeaponWoosh weaponWoosh;

    private void Awake()
    {
        grab = GetComponent<UnityEngine.XR.Interaction.Toolkit.Interactables.XRGrabInteractable>();
        weaponWoosh = GetComponent<VRWeaponWoosh>();
    }

    private void OnEnable()
    {
        grab.selectEntered.AddListener(OnGrab);
        grab.selectExited.AddListener(OnRelease);
    }

    private void OnDisable()
    {
        grab.selectEntered.RemoveListener(OnGrab);
        grab.selectExited.RemoveListener(OnRelease);
    }

    private void OnGrab(SelectEnterEventArgs args)
    {
        if (weaponWoosh == null) return;
        weaponWoosh.IsWeaponInHand(true);
    }

    private void OnRelease(SelectExitEventArgs args)
    {
        if (weaponWoosh == null) return;
        weaponWoosh.IsWeaponInHand(false);
    }
}