using UnityEngine;

public class AudioRoomsReflectionsMaster : MonoBehaviour
{
    public GameObject HallwayRoomReflections;
    public GameObject Room0Reflections;
    public GameObject Room1Reflections;
    public GameObject Room2Reflections;
    public GameObject Room3Reflections;
    
    
    
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
    HallwayRoomReflections.SetActive(true);
   
    }

    public void ActivateRoom1Reflections()
    {
        if (Room0Reflections!=null)
        {
        HallwayRoomReflections.SetActive(false);
        Room0Reflections.SetActive(true);
        Room1Reflections.SetActive(false);
        Room2Reflections.SetActive(false);
        Room3Reflections.SetActive(false);
      
        }
    }
    public void ActivateRoom1Refelctions()
    {
        if (Room2Reflections!= null)
        {
        HallwayRoomReflections.SetActive(false);
        Room0Reflections.SetActive(false);
        Room1Reflections.SetActive(true);
        Room2Reflections.SetActive(false);
        Room3Reflections.SetActive(false);
       
        }
    }
    
    public void ActivateRoom2Reflections()
    {
        if (Room3Reflections != null)
        {
        HallwayRoomReflections.SetActive(false);
        Room0Reflections.SetActive(false);
        Room1Reflections.SetActive(false);
        Room2Reflections.SetActive(true);
        Room3Reflections.SetActive(false);
      
        }
    }
    public void ActivateRoom3Reflections()
    {
        if (Room3Reflections != null)
        {
        HallwayRoomReflections.SetActive(false);
        Room0Reflections.SetActive(false);
        Room1Reflections.SetActive(false);
        Room2Reflections.SetActive(false);
        Room3Reflections.SetActive(true);
       
        }
    }
   
    
    
    // Update is called once per frame
    void Update()
    {
        
    }
}
