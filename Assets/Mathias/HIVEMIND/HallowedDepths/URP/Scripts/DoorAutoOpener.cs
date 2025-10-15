using UnityEngine;

public class DoorAutoOpener : MonoBehaviour
{
    [Header("Door Animator")]
    public Animator doorAnimator;

    [Header("Player Tag")]
    public string playerTag = "Player";

    private void Start()
    {
        // Keep the door closed at the start of the game
        doorAnimator.SetBool("IsOpen", false);
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag(playerTag))
        {
            Debug.Log("Door opening");
            doorAnimator.SetBool("IsOpen", true);
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (other.CompareTag(playerTag))
        {
            Debug.Log("Door closing");
            doorAnimator.SetBool("IsOpen", false);
        }
    }
}
