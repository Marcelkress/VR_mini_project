using UnityEngine;

public class NERoomsManager : MonoBehaviour
{
    public GameObject[] rooms;
    public GameObject corridor;
    
    private int lastRoomIndex;
    
    public void EnteredRoom(int roomIndex)
    {
        if (roomIndex > lastRoomIndex)
        {
            if (roomIndex == rooms.Length - 1)
            {
                corridor.SetActive(true);
            }
            else
            {
                rooms[roomIndex + 1].SetActive(true);
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
            lastRoomIndex = roomIndex;
        }
    }
    
}