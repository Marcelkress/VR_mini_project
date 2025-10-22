using System.Collections;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.XR.Interaction.Toolkit.Interactors;

public class TeleportController : MonoBehaviour
{
    public InputActionProperty teleportActivateAction;
    public XRRayInteractor teleportInteractor;
    
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        teleportInteractor.gameObject.SetActive(false);

        teleportActivateAction.action.Enable();
        teleportActivateAction.action.performed += ActionOnperformed;
        teleportActivateAction.action.canceled += ActionOncanceled;
    }

    private void ActionOncanceled(InputAction.CallbackContext obj)
    {
        StartCoroutine(SkipOneFrame());
    }

    private void ActionOnperformed(InputAction.CallbackContext obj)
    {
        teleportInteractor.gameObject.SetActive(true);
    }

    private IEnumerator SkipOneFrame()
    {
        yield return null;
        teleportInteractor.gameObject.SetActive(false);
    }
}
