using System;
using UnityEngine;


public class AudioRoomScript : MonoBehaviour
{
    public AudioRoomsReflectionsMaster audioroomReflections;
    
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    private void OnTriggerEnter(Collider other)

    {
        if (other.gameObject.CompareTag("Room 0 og 4"))
        {
            audioroomReflections.ActivateRoom0Reflections();
        }
        
        if (other.gameObject.CompareTag("Room 1 og 5"))
        {
            audioroomReflections.ActivateRoom1Reflections();
        }
        
        if (other.gameObject.CompareTag("Room 2"))
        {
            audioroomReflections.ActivateRoom2Reflections();
        }
        if (other.gameObject.CompareTag("Room 3"))
        {
            audioroomReflections.ActivateRoom3Reflections();
        }
        
        if (other.gameObject.CompareTag("Hallway"))
        {
            audioroomReflections.ActivateHallwayReflections();
        }
        
    }
    
   
}
