using UnityEngine;
using UnityEngine.InputSystem;

public class HandAnimationControler : MonoBehaviour

{
    // Assign the InputAction from your Input Action Asset in the Inspector
    // e.g. XRI LeftHand Activate or XRI RightHand Activate
    [SerializeField] private InputActionProperty activateAction;

    public Animator handAnimator;


    void Update()
    {

            float value = activateAction.action.ReadValue<float>();
            handAnimator.SetFloat("Grab", value);
            // ‘value’ is 0.0 (unpressed) to 1.0 (fully pressed)
            Debug.Log("Activate value: " + value);
    }
}


