using UnityEngine;
using UnityEngine.XR;
using UnityEngine.XR.Interaction.Toolkit;

[RequireComponent(typeof(UnityEngine.XR.Interaction.Toolkit.Interactables.XRGrabInteractable))]
public class WeaponHandBinder : MonoBehaviour
{
    private UnityEngine.XR.Interaction.Toolkit.Interactables.XRGrabInteractable grab;
    private VRWeaponWoosh2 woosh;

    private void Awake()
    {
        grab = GetComponent<UnityEngine.XR.Interaction.Toolkit.Interactables.XRGrabInteractable>();
        woosh = GetComponent<VRWeaponWoosh2>();
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
        if (woosh == null)
            return;

        // Name of the interactor object (hand controller)
        string interactorName = args.interactorObject.transform.name.ToLower();

        // Decide hand based on naming convention
        if (interactorName.Contains("left"))
        {
            woosh.SetHand(XRNode.LeftHand);
        }
        else if (interactorName.Contains("right"))
        {
            woosh.SetHand(XRNode.RightHand);
        }
        else
        {
            // Fallback: default to right if it doesn't contain left/right
            woosh.SetHand(XRNode.RightHand);
        }

        woosh.IsWeaponInHand(true);
    }

    private void OnRelease(SelectExitEventArgs args)
    {
        if (woosh == null)
            return;

        woosh.IsWeaponInHand(false);
    }
}