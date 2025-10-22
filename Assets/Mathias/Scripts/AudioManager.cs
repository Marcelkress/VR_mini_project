
using UnityEngine;
using FMODUnity;
using FMOD.Studio;

public class AudioManager : MonoBehaviour
{
    
    [field: SerializeField] private FmodEvents events;
    public static AudioManager Instance { get; private set; }
    FMOD.Studio.EventInstance backgroundMusic;

    private void Awake()
    {
        if (Instance != null)
        {
            Debug.LogError("Found more than one AudioManager in the scene");
            Destroy(gameObject);
            return;
        }

        Instance = this;
    }

    public void Start()
    {
        
            backgroundMusic = FMODUnity.RuntimeManager.CreateInstance(FmodEvents.Instance.Fightmusic);
            backgroundMusic.start();
         

    }
    public void playOneShot(string eventPath, Vector3 worldPosition)
    {
        RuntimeManager.PlayOneShot(eventPath, worldPosition);
    }
    
    
}