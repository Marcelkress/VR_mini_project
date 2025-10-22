using UnityEngine;
using UnityEngine.InputSystem;

public class HandAnimationControler : MonoBehaviour
{
    [SerializeField] private InputActionProperty selectAction;

    public Animator handAnimator;
    void Update()
    {
        float value = selectAction.action.ReadValue<float>();
        handAnimator.SetFloat("Grab", value);
    }
}


