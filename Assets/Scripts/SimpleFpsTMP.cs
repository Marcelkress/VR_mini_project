using TMPro;
using UnityEngine;

public class SimpleFpsTMP : MonoBehaviour
{
    [Header("Assign a TMP Text (TextMeshProUGUI or TMP_Text in world-space)")]
    public TMP_Text label;

    [Range(0, 3)] public int fpsDecimals = 0;
    [Tooltip("Text updates per second (lower = fewer updates/allocs).")]
    public float refreshRate = 4f;
    [Tooltip("EMA smoothing for FPS (0 = raw, 0.9 is smooth).")]
    [Range(0f, 0.99f)] public float smoothing = 0.9f;

    float emaDelta = 1f / 60f;
    float nextUpdate;

    void Reset() => label = GetComponent<TMP_Text>();

    void Update()
    {
        // Smooth delta (EMA)
        emaDelta = Mathf.Lerp(emaDelta, Time.unscaledDeltaTime, 1f - smoothing);

        if (Time.unscaledTime < nextUpdate) return;
        nextUpdate = Time.unscaledTime + 1f / Mathf.Max(1f, refreshRate);

        float fps = 1f / Mathf.Max(1e-6f, emaDelta);
        float ms = emaDelta * 1000f;

        if (label)
            label.text = $"FPS: {fps.ToString($"F{fpsDecimals}")}  ({ms.ToString("F1")} ms)";
    }
}