using FMOD.Studio;
using FMODUnity;
using UnityEngine;

public class FightMusic : MonoBehaviour
{
    FmodEvents fmodEvents;
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void StartFightMusic()
    {
        fmodEvents = FmodEvents.Instance;
        EventInstance musicInstance = RuntimeManager.CreateInstance(fmodEvents.Fightmusic);
        musicInstance.start();
    }
    
    
    
    
}
