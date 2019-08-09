# SecureDesk

## Description
On Windows systems, desktops are separated by the kernel; window messages can be sent only between processes that are on the same desktop.

Particularly, the hook procedures (abundantly used by keyloggers) can only receive messages intended for windows created in the same desktop.

Thus, virtual desktops can be employed as a provisional protection technique against spyware like keyloggers and screen recorders.
This is a comparable mechanism to the one used in the Ctrl+Alt+Del and the WinLogon screens.


## Controls
After execution, you can switch between the original and the secure desktop.

- **Ctrl + Alt + RightArrow** : Switch to secure hidden desktop
- **Ctrl + Alt + LeftArrow**  : Switch to original desktop
- **Ctrl + C**                : Exit program

