using UnityEngine;
using UnityEngine.InputSystem;
using FMODUnity;
using UnityEngine.XR.Interaction.Toolkit;


public class SimpleFootstep : MonoBehaviour
{
    [SerializeField] private InputActionProperty leftThumbstick; // assign your LeftHand Move action
    [SerializeField] private StudioEventEmitter footstepEmitter;
    [SerializeField] private float stepInterval = 0.5f; // seconds between steps when moving
    [SerializeField] private float moveDeadzone = 0.1f;

    private float stepTimer;

    void OnEnable()
    {
        leftThumbstick.action.Enable();
    }
    
    void OnDisable()
    {
        leftThumbstick.action.Disable();
    }

    void Update()
    {
        Vector2 input = leftThumbstick.action.ReadValue<Vector2>();
        bool isMoving = input.magnitude > moveDeadzone;

        if (isMoving)
        {
            stepTimer -= Time.deltaTime;
            if (stepTimer <= 0f)
            {
                footstepEmitter.Play();
                stepTimer = stepInterval;
            }
        }
        else
        {
            stepTimer = 0f; // reset timer when not moving
        }
    }
}