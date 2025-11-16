using UnityEngine;
using FMODUnity;

public class PotionSoundScript : MonoBehaviour
{
    [Header("Hit Detection")]
    public float radius = 0.05f;

    [Header("Tag to detect (e.g. 'Floor')")]
    public string floorTag = "Floor";

    [Header("Potion FMOD")]
    public StudioEventEmitter potionSoundFloor;
    public EventReference glassSound;
    public GameObject hitSoundPosition;

    private Vector3 _lastPosition;

    void Start()
    {
        _lastPosition = transform.position;
    }

    void LateUpdate()
    {
        Vector3 currentPos = transform.position;
        Vector3 delta = currentPos - _lastPosition;
        float distance = delta.magnitude;

        if (distance > 0.0001f)
        {
            RaycastHit hit;
            Vector3 direction = delta.normalized;

            if (Physics.SphereCast(
                    _lastPosition,
                    radius,
                    direction,
                    out hit,
                    distance,
                    Physics.DefaultRaycastLayers,
                    QueryTriggerInteraction.Ignore))
            {
                // Hit something — is it the Floor?
                if (hit.collider.CompareTag(floorTag))
                {
                    PotionOnFloor(hit);
                }
            }
        }

        _lastPosition = currentPos;
    }

    private void PotionOnFloor(RaycastHit hit)
    {
        Vector3 pos = hitSoundPosition != null ? hitSoundPosition.transform.position : hit.point;
        RuntimeManager.PlayOneShot(glassSound, pos);
    }
}