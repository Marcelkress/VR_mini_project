using UnityEngine;
using System.Collections;

public class THC1_ctrl : MonoBehaviour {
	
	
	private Animator anim;
	private CharacterController controller;
	private int battle_state = 0;
	public float speed = 6.0f;
	public float runSpeed = 3.0f;
	public float turnSpeed = 60.0f;
	public float gravity = 20.0f;
	private Vector3 moveDirection = Vector3.zero;
	private float w_sp = 0.0f;
	private float r_sp = 0.0f;

	
	// Use this for initialization
	void Start () 
	{						
		anim = GetComponent<Animator>();
		controller = GetComponent<CharacterController> ();

		//w_sp = speed; //read walk speed
		r_sp = runSpeed; //read run speed
	}
	
	// Update is called once per frame
	void Update () 
	{		
		if (Input.GetKey ("1"))  // turn to battle state with walking
		{ 		
			anim.SetInteger ("battle", 0);
			battle_state = 0;
		}
		if (Input.GetKey ("2")) // turn to battle state with run
		{ 
			anim.SetInteger ("battle", 1);
			battle_state = 1;
			
		}
		if (Input.GetKey ("3")) // turn to crawl stay
		{ 
			anim.SetInteger ("battle", 2);
			battle_state = 2;
			
		}
		if (Input.GetKey ("4")) // turn to SLLEP state
		{ 
			anim.SetInteger ("battle", 3);
			battle_state = 3;
			
		}
			
		if (Input.GetKey ("up")) 
		{	
			if (battle_state == 0) {
				anim.SetInteger ("moving", 1);//walk
				runSpeed = 1;
			}

			if (battle_state == 1) {
				anim.SetInteger ("moving", 2);//run
				runSpeed = r_sp;
			}
			if (battle_state == 2) {	
				anim.SetInteger ("moving", 3);//crawl
				runSpeed = 0.66f;
			}
			if (battle_state == 3) {	//sleep - no MOVES

				runSpeed = 0;
			}
		}
		else 
			{
				anim.SetInteger ("moving", 0);
			}

	
		if (Input.GetMouseButtonDown (0)) { // attack1
			anim.SetInteger ("moving", 4);
		}
		if (Input.GetMouseButtonDown (1)) { // attack2
			anim.SetInteger ("moving", 5);
		}
		if (Input.GetMouseButtonDown (2)) { // attack3
			anim.SetInteger ("moving", 6);
		}

		if (Input.GetKeyDown ("i")) //die_1
		{ 
			anim.SetInteger ("moving", 13);
		}

		if (Input.GetKeyDown ("o")) //die_2
		{ 
			anim.SetInteger ("moving", 12);
		}
		
		if (Input.GetKeyDown ("u")) //hit
		{   
				int n = Random.Range (0, 2);
				if (n == 0) 
				{
					anim.SetInteger ("moving", 10);
				} 
				else 
				{
				anim.SetInteger ("moving", 11);
				}
		}

		if (Input.GetKeyDown ("p")) { // defence_start
			anim.SetInteger ("moving", 14);
		}
		if (Input.GetKeyUp ("p")) { // defence_end
			anim.SetInteger ("moving", 15);
		} 

		if (Input.GetKeyDown ("z")) { // eating
			anim.SetInteger ("moving", 17);
		}
		if (Input.GetKeyUp ("z")) { // eating_end
			anim.SetInteger ("moving", 0);
		} 

		if (Input.GetKeyDown ("x")) { //bite
			anim.SetInteger ("moving",7);
		}
		if (Input.GetKeyDown ("c")) { //roar
			anim.SetInteger ("moving", 8);
		}
		if (Input.GetKeyDown ("space")) { //jump
			anim.SetInteger ("moving", 16);
		}
	
		if (Input.GetKeyDown ("v")) { //crawl_bite
			anim.SetInteger ("moving", 18);
		}



		if (controller.isGrounded) 
		{
			moveDirection=transform.forward * Input.GetAxis ("Vertical") * speed * runSpeed;
			float turn = Input.GetAxis("Horizontal");
			transform.Rotate(0, turn * turnSpeed * Time.deltaTime, 0);						
		}
		moveDirection.y -= gravity * Time.deltaTime;
		controller.Move (moveDirection * Time.deltaTime);
		}
}



