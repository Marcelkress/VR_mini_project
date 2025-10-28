using UnityEngine;

public class PortalCamera : MonoBehaviour
{
    public Transform playerCamera;
    public Transform portal;
    public Transform otherPortal;

    void LateUpdate()
    {
       Vector3 playerOffsetFromPortal = playerCamera.position - otherPortal.position;
       transform.position = portal.position + playerOffsetFromPortal;

       float angularDifferenceBetweenPortalRotations = Quaternion.Angle(portal.rotation, otherPortal.rotation);

       Quaternion portalRotationalDifference = Quaternion.AngleAxis(angularDifferenceBetweenPortalRotations, Vector3.up);
       Vector3 newCameraDirection = portalRotationalDifference * playerCamera.forward;
       transform.rotation = Quaternion.LookRotation(newCameraDirection, Vector3.up);

      // TODO: 
      // - Vi skal fikse rotation p� x-aksen
      // - Ting skal vidst i Late Update, s� den opdaterer mere korrekt if�lge yt kommentarer.

    }

    /*
    void LateUpdate()
    {
        // Step 1: Convert player's position relative to other portal
        Vector3 playerRelativePosition = otherPortal.InverseTransformPoint(playerCamera.position);

        // Step 2: Apply that relative position to the current portal
        transform.position = portal.TransformPoint(playerRelativePosition);

        // Step 3: Convert player's rotation to other portal's local space
        Quaternion playerRelativeRotation = Quaternion.Inverse(otherPortal.rotation) * playerCamera.rotation;

        // Step 4: Apply that rotation to current portal's rotation
        transform.rotation = portal.rotation * playerRelativeRotation;
    }
    */
}
