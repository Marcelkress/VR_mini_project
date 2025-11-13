using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using FMODUnity; 

public class FmodEvents : MonoBehaviour
{
    //This script is used to store all the FMOD events that are used in the game.
    //We create a public static instance of the FmodEvents script so that we can access it from anywhere in the game.
    //We also create a private set so that we can only set the instance in this script.
    //We then create a private Awake method that checks if the instance is not null and if it is it will log an error message.
    [field: Header("Skeleton damage")]
    [field: SerializeField] public EventReference SkeletonDamage { get; private set; }
    
    [field: Header ("Skeleton breath")]
    [field: SerializeField] public EventReference SkeletonBreath { get; private set; }
    
    [field: Header ("Fight music")]
    [field: SerializeField] public EventReference Fightmusic { get; private set; }
    
   // [field: Header ("Monster attack")]
   // [field: SerializeField] public EventReference MonsterAttack { get; private set; }
    
    
    
    
    //We then create a public static instance of the FmodEvents script so that we can access it from anywhere in the game.
    //We also create a private set so that we can only set the instance in this script.
    public static FmodEvents Instance { get; private set; } 
   
    private void Awake()
    {
        if (Instance != null)
        {
            Debug.LogError("Found more than one FMOD Events scripts in the scene");
        }
       
        Instance = this;
    }
}
