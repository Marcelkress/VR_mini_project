using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using FMOD.Studio;
using FMODUnity;
using FMOD.Studio;

public class FmodAudioManager : MonoBehaviour
{
 
    private List<EventInstance> _eventInstances;

    //Creating a singleton pattern for the FmodAudioManager so that we can access
    //it from anywhere in the game and check if it already exists making sure we only have one instance of it.
    //This is done by creating a public static instance of the FmodAudioManager and a private set so that we can only set it in this script.
    //We then create a private Awake method that checks if the instance is not null and if it is it will log an error message.
    //If the instance is null it will set the instance to this script and create a new list of EventInstances.
    //This list will be used to store all the EventInstances that are created during the game.
    //This is done so that we can stop all the sounds when the game is paused or when the player dies.
    
    public static FmodAudioManager Instance { get; private set; }

    private void Awake()
    {
        if (Instance != null)
        {
            Debug.LogError("Found more than one FModAudioManager in the scene");
        }


        Instance = this;
        _eventInstances = new List<EventInstance>();
    }

    
    
    //This method is used to play a sound once at a specific position in the world.
    public void playOneShot(EventReference sound, Vector3 worldPos)
    {
        RuntimeManager.PlayOneShot(sound, worldPos);
        
    }

    public EventInstance CreateEventInstance(EventReference eventReference) //This method is used to create an EventInstance from an EventReference.
    {
        EventInstance eventInstance = RuntimeManager.CreateInstance(eventReference); //We create a new EventInstance from the EventReference.
        return eventInstance;
    }
    
   
    void OnDestroy()
    {
        Cleanup();
    }

    private void Cleanup()
    {
        foreach (EventInstance eventInstance in _eventInstances)
        {
            eventInstance.stop(FMOD.Studio.STOP_MODE.IMMEDIATE);
            eventInstance.release();
        }
    }
}
