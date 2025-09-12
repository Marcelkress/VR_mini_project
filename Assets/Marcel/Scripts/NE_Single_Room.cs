using System;
using UnityEngine;

public class NE_Single_Room : MonoBehaviour
{
    public NERoomsManager RoomsManager;
    public int roomID;
    
    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("Player"))
        {
            RoomsManager.EnteredRoom(roomID);    
        }
    }
}
