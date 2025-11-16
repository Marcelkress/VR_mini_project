using UnityEngine;
using FMOD.Studio;
using FMODUnity;
using FMODUnity;

public class DragonScream : MonoBehaviour
{
   public StudioEventEmitter dragonScreamEmitter;
   public bool hasbeenCalled = false;
   public void PlayDragonScream()
   {
       if (dragonScreamEmitter != null && hasbeenCalled==false)
       {
           dragonScreamEmitter.Play();
           hasbeenCalled = true;
       
       }
   } 
      public void OnTriggerEnter(Collider other)
    {
         if (other.CompareTag("Player"))
         {
              PlayDragonScream();
         }
    }

   

}
