using UnityEngine;
using UnityEngine.XR;
using UnityEngine.XR.Interaction.Toolkit;

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
        if (grab == null) return;
        grab.selectEntered.AddListener(OnGrab);
        grab.selectExited.AddListener(OnRelease);
    }

    private void OnDisable()
    {
        if (grab == null) return;
        grab.selectEntered.RemoveListener(OnGrab);
        grab.selectExited.RemoveListener(OnRelease);
    }

    private void OnGrab(SelectEnterEventArgs args)
    {
        if (weaponWoosh == null) return;

        // Get the interactor GameObject (the controller / hand)
        var interactorGO = args.interactorObject.transform.gameObject;

        XRNode node = XRNode.RightHand;

        // Try to get XRController (Action-based or Device-based)
        var xrController = interactorGO.GetComponent<XRController>();
        if (xrController != null)
        {
            node = xrController.controllerNode;   // This will be LeftHand or RightHand
        }
        else
        {
            // Fallback: infer from object name if no XRController
            string lowerName = interactorGO.name.ToLower();
            if (lowerName.Contains("left"))
                node = XRNode.LeftHand;
            else
                node = XRNode.RightHand;
        }

        weaponWoosh.SetHand(node);
        weaponWoosh.IsWeaponInHand(true);
    }

    private void OnRelease(SelectExitEventArgs args)
    {
        if (weaponWoosh == null) return;
        weaponWoosh.IsWeaponInHand(false);
    }
}