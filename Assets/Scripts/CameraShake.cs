using UnityEngine;
using System.Collections;
using UnityEngine.XR.Interaction.Toolkit;
using UnityEngine.XR.Interaction.Toolkit.Inputs.Haptics;

public class CameraShake : MonoBehaviour
{
    public static CameraShake instance;

    private Vector3 originalLocalPosition;
    private Transform shakeTransform;  
    private Coroutine shakeCoroutine;

    public HapticImpulsePlayer hapticImpulsePlayerLeft, hapticImpulsePlayerRight;

    private void Awake()
    {
        if (instance == null)
        {
            instance = this;
            shakeTransform = this.transform;
            originalLocalPosition = shakeTransform.localPosition;
        }
        else
        {
            Destroy(gameObject);
        }
  }

    public void TriggerShake(float duration, float magnitude)
    {
        if (shakeCoroutine != null)
        {
            StopCoroutine(shakeCoroutine);
            shakeTransform.localPosition = originalLocalPosition;
        }
        shakeCoroutine = StartCoroutine(ShakeCoroutine(duration, magnitude));
        VibrateController(duration, 0.5f, 0f);
    }

    private IEnumerator ShakeCoroutine(float duration, float magnitude)
    {
        float elapsed = 0.0f;

        while (elapsed < duration)
        {
            // POSITIONAL SHAKE ONLY - NO ROTATION
            float x = Random.Range(-1f, 1f) * magnitude;
            float y = Random.Range(-1f, 1f) * magnitude;

            // Apply offset to the container, not the camera directly
            shakeTransform.localPosition = originalLocalPosition + new Vector3(x, y, 0);

            elapsed += Time.deltaTime;
            yield return null;
        }

        // Return to center
        shakeTransform.localPosition = originalLocalPosition;
    }

    private void VibrateController(float duration, float frequency, float amplitude)
    {
       foreach (var hapticPlayer in new[] { hapticImpulsePlayerLeft, hapticImpulsePlayerRight })
       {
           if (hapticPlayer != null)
           {
               hapticPlayer.SendHapticImpulse(frequency, amplitude, duration);
           }
       }
    }
}
