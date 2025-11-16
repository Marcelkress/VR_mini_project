using UnityEngine;
using FMODUnity;
using FMOD.Studio;
using System.Collections;

public class MusicFade : MonoBehaviour
{
    [Header("FMOD Emitters")]
    public StudioEventEmitter currentTrack;
    public StudioEventEmitter nextTrack;
    public StudioEventEmitter banshee;

    [Header("Timing")]
    public float fadeOutTime = 2f;   // time to fade out current track
    public float delayTime = 3f;     // wait time before new track starts
    public bool hasbeenCalled = false;
    private EventInstance currentInstance;

    void Start()
    {
        // Cache the instance for volume control
        if (currentTrack != null)
            currentInstance = currentTrack.EventInstance;
    }

    public void FadeOutAndSwitch()
    {
        if (hasbeenCalled==false)
        {
            hasbeenCalled = true;
            StartCoroutine(FadeOutAndStartNewTrack());
        }
        
     
    }

    private IEnumerator FadeOutAndStartNewTrack()
    {
        // ---- 1. Fade out current track ----
        float timer = 0f;

        currentInstance.getVolume(out float startingVolume);

        while (timer < fadeOutTime)
        {
            timer += Time.deltaTime;
            float t = timer / fadeOutTime;
            float newVolume = Mathf.Lerp(startingVolume, 0f, t);

            currentInstance.setVolume(newVolume);
           // banshee.Play();
            yield return null;
        }

        currentInstance.setVolume(0f);
        currentInstance.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
     
        // ---- 2. Wait for delayTime seconds ----
        yield return new WaitForSeconds(delayTime);
        

        // ---- 3. Start new FMOD track ----
        if (nextTrack != null)
        {
            nextTrack.Play();
        }
    }
}

