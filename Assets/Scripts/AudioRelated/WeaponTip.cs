using UnityEngine;
using FMODUnity;

public class WeaponTip : MonoBehaviour
{
    [Header("Hit Detection")]
    public float radius = 0.05f;       // how "thick" the sweep is
    public string monsterTag = "Monster";

    [Header("Monster FMOD")]
    public StudioEventEmitter monsterHitEmitter; // optional generic monster impact
    public EventReference BloodyHit;
    public GameObject hitSoundPosition; // optional (fallback = hit point)

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
            RaycastHit hit = new RaycastHit();
            Vector3 direction = delta.normalized;

            if (Physics.SphereCast(_lastPosition, radius, direction, out hit, distance))
            {
                // ---- If door, let door script handle it ----
                DoorHitSound door = hit.collider.GetComponentInParent<DoorHitSound>();
                if (door != null)
                {
                    door.OnHit(hit);
                }
                // ---- If Monster, handle here directly ---
                else if (hit.collider.CompareTag(monsterTag))
                {
                    HandleMonsterHit(hit);
                }
            }
        }

        _lastPosition = currentPos;
    }

    private void HandleMonsterHit(RaycastHit hit)
    {
        if (monsterHitEmitter != null)
            monsterHitEmitter.Play();

        if (hitSoundPosition != null)
        {
            RuntimeManager.PlayOneShotAttached(BloodyHit, hitSoundPosition);
        }
        else
        {
            RuntimeManager.PlayOneShot(BloodyHit, hit.point);
        }
    }
}