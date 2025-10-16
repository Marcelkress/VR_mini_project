using UnityEngine;
using FMODUnity;
using FMOD.Studio;
using static FmodEvents;

public class TEstClickSound : MonoBehaviour
{
    public void TestClickSound()
    {
        Clicksound = FMODUnity.RuntimeManager.CreateInstance(FmodEvents.Instance.TestClick);
        Clicksound.set3DAttributes(FMODUnity.RuntimeUtils.To3DAttributes(gameObject));
        Clicksound.start();
    }
    
    FMOD.Studio.EventInstance Clicksound;
    
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        
        TestClickSound();
    }

    // Update is called once per frame
    void Update()
    {
        Clicksound.set3DAttributes(FMODUnity.RuntimeUtils.To3DAttributes(gameObject));
    }
}
