using UnityEngine;

public class GentleSwing : MonoBehaviour
{
    [Header("Swing (degrees)")]
    public float maxAngle = 5f;     // peak swing angle
    public float speed = 0.6f;      // swing cycles per second
    public float noise = 0.3f;      // adds subtle randomness

    [Header("Axis (local)")]
    public Vector3 localAxis = new Vector3(0, 1, 0); // e.g. swing around Y

    Quaternion _baseRot;
    float _seed;

    void Awake()
    {
        _baseRot = transform.localRotation;
        _seed = Random.value * 1000f;
        localAxis = localAxis.normalized;
    }

    void Update()
    {
        float t = Time.time;
        float sine = Mathf.Sin(t * Mathf.PI * 2f * speed);
        float wiggle = (Mathf.PerlinNoise(_seed, t * speed) - 0.5f) * 2f * noise;
        float angle = (sine + wiggle) * maxAngle;

        var swing = Quaternion.AngleAxis(angle, localAxis);
        transform.localRotation = _baseRot * swing;
    }
}

