using UnityEngine;

public class PortalTeleporter : MonoBehaviour
{
    public Transform player;

    //Receiving portal
    public Transform receiver;

    private bool playerIsOverlapping = false;

    // Update is called once per frame
    void Update()
    {
        if (playerIsOverlapping)
        {
            Vector3 portalToPlayer = player.position - transform.position;
            float dotProduct = Vector3.Dot(transform.up, portalToPlayer);

            // If this is true, the player has moved across the portal
            if (dotProduct < 0f)
            {
                float rotationDiff = Quaternion.Angle(transform.rotation, receiver.rotation);

                // Det her er til specifikt det her scenarie, da vi ved hvordan at portalerne er placeret.
                //TODO: Skal generaliseres
                rotationDiff += 180;
                player.Rotate(Vector3.up, rotationDiff);

                Vector3 positionOffset = Quaternion.Euler(0f, rotationDiff, 0f) * portalToPlayer;
                player.position = receiver.position + positionOffset;

                playerIsOverlapping = false;
            }
        }

    }

    void OnTriggerEnter(Collider other)
    {
        if (other.tag == "Player")
        {
            playerIsOverlapping = true;
        }
    }

    void OnTriggerExit(Collider other)
    {
        if (other.tag == "Player")
        {
            playerIsOverlapping = false;
        }
    }
}
