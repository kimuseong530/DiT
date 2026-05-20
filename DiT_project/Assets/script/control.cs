using System.Collections;
using System.Collections.Generic;
using UnityEngine;



public class control : MonoBehaviour
{
    public ArticulationBody front_right;
    public ArticulationBody front_left;
    public ArticulationBody back_right;
    public ArticulationBody back_left;

    void Update()
    {
        // 1번 키 → 앞
        if (Input.GetKeyDown(KeyCode.Alpha1))
        {
            front_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, 100);
            front_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, 100);
            back_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, 100);
            back_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, 100);
        }

        // 2번 키 → 앞 오른쪽
        if (Input.GetKeyDown(KeyCode.Alpha2))
        {
            front_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, 50);
            front_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, 100);
            back_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, 50);
            back_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, 100);
        }

        // 3번 키 → 앞 왼쪽
        if (Input.GetKeyDown(KeyCode.Alpha3))
        {
            front_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, 100);
            front_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, 50);
            back_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, 100);
            back_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, 50);
        }

        // 4번 키 → 뒤로가기
        if (Input.GetKeyDown(KeyCode.Alpha4))
        {
            front_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, -100);
            front_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, -100);
            back_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, -100);
            back_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, -100);
        }

        // 4번 키 → 멈추기
        if (Input.GetKeyDown(KeyCode.Alpha5))
        {
            front_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, 0);
            front_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, 0);
            back_right.SetDriveTargetVelocity(ArticulationDriveAxis.X, 0);
            back_left.SetDriveTargetVelocity(ArticulationDriveAxis.X, 0);
        }
    }
}