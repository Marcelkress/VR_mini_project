using System;
using UnityEngine;

public class NERoomsManager : MonoBehaviour
{
    public GameObject[] rooms;
    public GameObject corridor;
    
    public int lastRoomIndex;

    private void Start()
    {
        lastRoomIndex = -1;

        foreach (var room in rooms)
        {
            room.SetActive(false);
        }
        
        rooms[rooms.Length - 1].SetActive(true);
        rooms[0].SetActive(true);
    }

    public void EnteredRoom(int roomIndex)
    {
        if (roomIndex > lastRoomIndex)
        {
            // Turn on corridor when reaching end
            if (roomIndex == rooms.Length - 1)
            {
                corridor.SetActive(true);
            }

            if (roomIndex == 0)
            {
                rooms[rooms.Length - 1].SetActive(false);
            }

            if (roomIndex == 1)
            {
                corridor.SetActive(false);
            }

            // turn on the next room ahead
            if (roomIndex < rooms.Length - 1)
            {
                Debug.Log("Room index: " + roomIndex);
                
                
                rooms[roomIndex + 1].SetActive(true);
            }

            // Turn off room behind player
            if (roomIndex - 2 >= 0)
            { 
                rooms[roomIndex - 2].SetActive(false);
            }
            
            lastRoomIndex = roomIndex;
        }
        
        if (roomIndex < lastRoomIndex)
        {
            if (roomIndex == 0)
            {
                corridor.SetActive(true);
            }
            else
            {
                rooms[roomIndex - 1].SetActive(true);
            }
            
            if (roomIndex + 2 <= rooms.Length)
            {
                rooms[roomIndex + 2].SetActive(false);
            }
            
            lastRoomIndex = roomIndex;
        }
        
        
    }
    
}