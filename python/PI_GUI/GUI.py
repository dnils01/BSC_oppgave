import serial
import time
import threading
import tkinter as tk
import tkinter.messagebox
import customtkinter
from customtkinter import *
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
from matplotlib.figure import Figure
import re
LIGHT_GRAY = "#F5F5F5"
LABEL_COLOR = "#25265E"
green= "#abf7b1"
red = "#ff6242"
flag = True
live_flag = False
Slow_decay_selected=1
Fast_decay_selected=0
position_previous=0
motor_running=0
max_speed=0
min_speed=0
melding=""
x_data=[]
y_data=[]
ref_data = []
data=np.array([])
figure, ax = plt.subplots() 
maaling =""
send_command_flag= False
command = ""
setpoint="0"
speed = "null"
Kp = ""
Ki= ""
Kd= ""
PID_flag= False
RPM_flag= False
Voltage_flag =False
read_serial_flag = True
changing_speed_flag=True
changing_load_flag=False
voltag_str=""

# Open the serial port
while True:
    try:
        ser = serial.Serial('/dev/ttyUSB0', 460800, timeout=0.5)
        ser.ReadBufferSize=1
        ser.flush()
        ser.reset_input_buffer()
        print("Kommunikasjon med serial opprettet")
        time.sleep(3)
        break 
    except serial.SerialException:
        print("Problemer med kommunikasjonen...prøver igjen")
        time.sleep(1)
customtkinter.set_appearance_mode("System")  # Modes: "System" (standard), "Dark", "Light"
customtkinter.set_default_color_theme("blue")  # Themes: "blue" (standard), "green", "dark-blue"      
root= customtkinter.CTk()
root.geometry(f"{800}x{400}")
# configure grid layout (4x4)
root.grid_columnconfigure(1, weight=1)
root.grid_columnconfigure((2, 3), weight=0)
root.grid_rowconfigure((0, 1, 2), weight=1)
# create sidebar frame with widgets
sidebar_frame = customtkinter.CTkFrame(root, width=140, corner_radius=0)
sidebar_frame.grid(row=0, column=0, rowspan=4, sticky="nsew")
sidebar_frame.grid_rowconfigure(4, weight=1)
textbox = customtkinter.CTkTextbox(root, width=250, font=("Helvetica", 24))
textbox.grid(row=0, column=1, padx=(20, 0), pady=(20, 0), sticky="nsew")
def exit_fullscreen():
    root.attributes("-fullscreen", False)  # Exit fullscreen
    root.unbind("<Escape>") 
#root.attributes("-fullscreen", True)
#root.bind("<Escape>", exit_fullscreen)
def Reset():
    global melding
    global melding,motor_running, max_speed, min_speed, setpoint
    progressbar_2.set(0)
    slider_1.set(0)
    setpoint= "0"
    melding=""
    motor_running=0
    max_speed=0
    min_speed=0
    textbox.delete("1.0","end")
    Stop_motor()
def send_command(comand,melding):
    global send_command_flag, command, read_serial_flag
    command = comand
    send_command_flag= True
    read_serial_flag = False
    print (command)
    textbox.delete("1.0","end")
    textbox.insert("0.0", "\n"  +melding)
   
def Minimum_speed():
    global melding, min_speed, max_speed
    if motor_running:
        min_speed=1
        max_speed=0
        comand="<D020>"
        melding= "Minimum speed"
        send_command(comand, melding)
    else:
        melding="motor is not running"
def Maximum_speed():
    global melding, max_speed, position_previous
    if motor_running:
        max_speed =1
        min_speed=0
        slider_1.set(1)
        progressbar_2.set(1)
        position_previous=1
        comand="<D100>"
        melding= "Maximum speed"
        send_command(comand, melding)
    else:
        melding = "motor is not running"
def Start_motor():
    global melding, motor_running
    if PID_flag and (int(float(setpoint))==0):
        melding = "Set a setpoint"
    elif PID_flag and (int(float(setpoint))!=0):
        melding = "Starting the motor with PID\nSetpoint="+setpoint
        comm1= "<D050>"
        motor_running=1
        send_command(comm1,melding)
        comm = "<y" + setpoint + ">"
        send_command(comm, melding)
    else:
        comm1= "<D050>"
        melding= "Starting the motor without PID"
        motor_running=1
        send_command(comm1,melding)
     
def Stop_motor():
    global melding,motor_running,max_speed, min_speed, position_previous
    progressbar_2.set(0)
    slider_1.set(0)
    position_previous=0
    if Slow_decay_selected==1:
        comand="<D000>"
        melding="Stopping the motor\nin the Slow_Decay mode"
    elif Fast_decay_selected==1:
        comand="<D000>"
        melding="Stopping the motor\nin the Fast_Decay mode"
    send_command(comand,melding )
    motor_running=0
    max_speed=0
    min_speed=0
def Stop_measuring():
    global flag, melding
    melding = "Stopped measuring"
    textbox.delete("1.0","end")
    textbox.insert("0.0", "\n\n Measuring Stopped")
    flag = False
def Start_measuring():
    global flag, melding, RPM_flag, Voltage_flag
    
    melding= "Start measuring"
    flag = True
    status=measuring.get()
    if flag:
        if status =="Motor":
            selected_parameter= optionmenu_Measu1._current_value
            if selected_parameter =="Speed(RPM)":
                RPM_flag = True
                Voltage_flag = False
                update_measurement()
                comand= "<MoRPM>"
                send_command(comand, melding)
            elif selected_parameter =="Voltage":
                RPM_flag = False
                Voltage_flag = True
                update_measurement()
                comand= "<MoVOL>"
                send_command(comand, melding)   
            else:
                textbox.delete("1.0","end")
                textbox.insert("0.0", "\n\n Measurements are not ready")
                flag= False
       
        elif status=="Generator":
            selected_parameter= optionmenu_Measu2._current_value
            textbox.delete("1.0","end")
            textbox.insert("0.0", "\n\n Generator Measurements are not ready")
            flag= False
        else:
            print("nothing")
def change_appearance_mode_event(new_appearance_mode: str):
    customtkinter.set_appearance_mode(new_appearance_mode)
def change_scaling_event(new_scaling: str):
    new_scaling_float = int(new_scaling.replace("%", "")) / 100
    customtkinter.set_widget_scaling(new_scaling_float)
def open_input_dialog_event():
    global melding, setpoint, Kp, Ki, Kd
    dialog = tk.Toplevel(root)
    dialog.title("Input Dialog")
    input_value = tk.StringVar()
    entry = tk.Entry(dialog, textvariable=input_value)
    entry.pack(padx=10, pady=10)
    def handle_click(key):
        global value
        if key == 'OK':
            value = entry.get()
            dialog.destroy()
        else:
            entry.insert(tk.END, key)

    def open_keyboard():
        keyboard = tk.Toplevel(dialog)
        keyboard.title("Digital Keyboard")
        buttons = [
            ('7', 0, 0), ('8', 0, 1), ('9', 0, 2),
            ('4', 1, 0), ('5', 1, 1), ('6', 1, 2),
            ('1', 2, 0), ('2', 2, 1), ('3', 2, 2),
            ('.', 3, 0),('0', 3, 1), ('OK', 3, 2)
        ]
        for (text, row, col) in buttons:
            btn = tk.Button(keyboard, text=text, width=5, height=2,
                            command=lambda t=text: handle_click(t))
            btn.grid(row=row, column=col, padx=5, pady=5)
    keyboard_button = tk.Button(dialog, text="Open Keyboard", command=open_keyboard)
    keyboard_button.pack(pady=5)
    ok_button = tk.Button(dialog, text="OK", command=lambda: handle_click('OK'))
    ok_button.pack(pady=5)
    dialog.wait_window()
    value = input_value.get()
    if value is not None:
        input_value.set(value)
    try:
        if value is not None:
            print(value)
            textbox.delete("1.0","end")
            selected_parameter= optionmenu_1._current_value
            print( selected_parameter)
            if selected_parameter == "Controller Parameters":
                textbox.insert("0.0", "Choose a parameter")
            elif selected_parameter == "Setpoint":
                melding= "Setpint="+ value
                print(melding)
                setpoint = value
                comm = "<y" + setpoint + ">"
                print(comm)
                send_command(comm, melding)
            elif selected_parameter == "Kp":
                Kp_flt= float(int(value)/100)
                Kp= str(Kp_flt)
                melding= "Kp="+ Kp
                print(melding)
                comm = "<p" + value + ">"
                send_command(comm, melding)
            elif selected_parameter == "Ki":
                Ki_flt= float(int(value)/100)
                Ki= str(Ki_flt)
                melding= "Ki="+ Ki
                print(melding)
                comm = "<i" + value + ">"
                send_command(comm, melding)
            elif selected_parameter == "Kd":
                Kd_flt= float(int(value)/100)
                Kd= str(Kd_flt)
                melding= "Kd="+ Kd
                print(melding)
                comm = "<d" + value + ">"
                send_command(comm, melding)
    except ValueError:
        melding= "Enter a valid value"
    
def slider_change(position_now):
    global melding
    progressbar_2.set(position_now)
    gain = str(int(position_now*4*25))
    print(gain)
    if changing_speed_flag and motor_running:
        melding= "Duty cycle="+gain+"%"
        if gain=="100":
            command="<D100>"
        elif gain=="0":
            command="<D000>"
        else:
            command="<D0"+gain+">"
        send_command(command, melding)
    elif  changing_speed_flag and not motor_running:
        melding= "Motor is not running"
    elif changing_load_flag:
        melding= "Load="+gain+"%"
        if gain=="100":
            command="<G100>"
        elif gain=="0":
            command="<G000>"
        else:
            command="<G0"+gain+">"
        send_command(command, melding)      
    print(command)
def Fast_Decay_event():
    global Fast_decay_selected, Slow_decay_selected
    Slow_decay.deselect()
    Fast_decay.select()
    Fast_decay_selected = Fast_decay.get()
    Slow_decay_selected = Slow_decay.get()
    com= "S0"
    meld= "Fast Decay"
    send_command(com, meld)
def Slow_Decay_event():
    global Fast_decay_selected, Slow_decay_selected
    Fast_decay.deselect()
    Slow_decay.select()
    Fast_decay_selected = Fast_decay.get()
    Slow_decay_selected = Slow_decay.get()
    com= "S1"
    meld= "Slow Decay"
    send_command(com, meld)
def Enable_PID_controller():
    global PID_flag, melding, setpoint
    status=PID_Enable.get()
    if status ==1:
        PID_flag =True
        melding= "PID-controller enabled"
        com= "<R1>"
        send_command(com, melding)
        print("PID enabled")
    elif status==0:
        PID_flag =False
        melding= "PID-controller disabled"
        setpoint= "0"
        com = "<R0>"
        send_command(com, melding)
        print("PID disabled")
def Analog_in_status():
    status=analog_in_switch.get()
    if status ==1:
        com="<A1>"
    else:
        com="<A0>"
    meld= "Analog input status changed"
    send_command(com, meld)
    
def Analog_out_status():
    status=analog_out_switch.get()
    if status ==1:
        com="<a1>"
    else:
        com="<a0>"
    meld= "Analog output status changed"
    send_command(com, meld)
def Digital_in_status():
    status=digital_in_switch.get()
    if status ==1:
        com="<U1>"
    else:
        com ="<U0>"
    meld= "Digital input status changed"
    send_command(com, meld)
    
def Digital_out_status():
    status=digital_out_switch.get()
    if status ==1:
        com="<U1>"
    else:
        com="<U0>"
    meld= "Digita< output status changed"
    send_command(com, meld)
def Changing_mode():
    global changing_speed_flag, changing_load_flag
    status=seg_button_1.get()
    if status=="Changing the Speed":
        changing_speed_flag=True
        changing_load_flag= False
    elif status =="Changing the Load":
        changing_load_flag=True
        changing_speed_flag= False
def Update_plot():
    global line, line2
    ax.set_xlabel('Time(second)')
    ax.set_ylabel('Speed(RPM)')
    ax.set_title('Step Respons')
    line.set_xdata(x_data)
 # Update plot
    line.set_ydata(y_data)
    line2.set_xdata(x_data)
    line2.set_ydata(ref_data)
 # Autoscale
    ax.relim()
    ax.autoscale_view()
 # Redraw canvas
    canvas.draw()

 # Call update_plot() again after 100 ms
    if live_flag:
        plot_window.after(100, Update_plot)
        
fig, ax = plt.subplots()
ax.set_xlabel('X-axis')
ax.set_ylabel('Y-axis')
ax.set_title('Dynamic Plot')
ax.grid()
line= ax.plot(x_data,y_data)[0]
line2 = ax.plot(x_data, ref_data)[0]
open_step_respons_flag= False
def Close_step_respons():
    global open_step_respons_flag
    open_step_respons_flag= False
    plot_window.destroy()
    plot_button.configure(text = "Open Step Respons")
def Freeze_step():
    global live_flag
    if live_flag:
        live_flag = False
        Toggle_button.configure(text="Start")
    else:
        live_flag= True
        Update_plot()
        Toggle_button.configure(text="Stop")
def Open_step_respons():
    global open_step_respons_flag, plot_flag
    global plot_window, fig, ax, canvas, x_data, y_data, line, live_flag, ref_data, line2
    live_flag = True
 
    if not open_step_respons_flag:

        global Toggle_button
        open_step_respons_flag = True
        plot_button.configure(text = "Close Step Respons")
        plot_window = tk.Toplevel(root)
        plot_window.title("Live Respons")
        
        fig = Figure(figsize=(5, 4), dpi=100)
        ax = fig.add_subplot(111)
        x_data=[]
        y_data=[]
        ref_data=[]
        line, = ax.plot(x_data, y_data, label= 'RPM')
        line2, = ax.plot(x_data, ref_data, label= 'SETPOINT')
        ax.legend()
        ax.grid()
        ax.set_ylim(0,4000)
        canvas = FigureCanvasTkAgg(fig, master=plot_window)
        canvas_tk =canvas.get_tk_widget()
        canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)
        
        close_button = tk.Button(plot_window, text="Close", command=Close_step_respons)
        close_button.pack()
        Toggle_button = tk.Button(plot_window, text="Stop", command=Freeze_step)
        Toggle_button.pack()
        Update_plot()

    else:
        Close_step_respons()
#buttons
sidebar_button_1 = customtkinter.CTkButton(sidebar_frame,text= "Start Motor", command=Start_motor)
sidebar_button_1.grid(row=0, column=0, padx=20, pady=10)
sidebar_button_2 = customtkinter.CTkButton(sidebar_frame,text="Stop Motor", command=Stop_motor)
sidebar_button_2.grid(row=1, column=0, padx=20, pady=10)
sidebar_button_3 = customtkinter.CTkButton(sidebar_frame,text="Minimum Speed", command=Minimum_speed)
sidebar_button_3.grid(row=2, column=0, padx=20, pady=10)
sidebar_button_4 = customtkinter.CTkButton(sidebar_frame,text= "Maximum Speed", command=Maximum_speed)
sidebar_button_4.grid(row=3, column=0, padx=20, pady=10)
appearance_mode_label = customtkinter.CTkLabel(sidebar_frame, text="Appearance Mode:", anchor="w")
appearance_mode_label.grid(row=5, column=0, padx=20, pady=(10, 0))
appearance_mode_optionemenu = customtkinter.CTkOptionMenu(sidebar_frame, values=["Light", "Dark", "System"],
                                                       command=change_appearance_mode_event)
appearance_mode_optionemenu.grid(row=6, column=0, padx=20, pady=(10, 10))
scaling_label = customtkinter.CTkLabel(sidebar_frame, text="UI Scaling:", anchor="w")
scaling_label.grid(row=7, column=0, padx=20, pady=(10, 0))
scaling_optionemenu = customtkinter.CTkOptionMenu(sidebar_frame, values=["70%", "80%", "90%", "100%", "120%"],
                                                      command=change_scaling_event)
scaling_optionemenu.grid(row=8, column=0, padx=20, pady=(10, 20))
# create main entry and button
main_button_1 = customtkinter.CTkButton(root,fg_color=("black", "lightgray"), text= "Reset", border_width=2)
main_button_1.grid(row=3, column=3, padx=(20, 20), pady=(20, 20), sticky="nsew")
# create tabview
tabview = customtkinter.CTkTabview(root, width=250)
tabview.grid(row=0, column=2, padx=(20, 0), pady=(20, 0), sticky="nsew")
tabview.add("PID Controller")
tabview.tab("PID Controller").grid_columnconfigure(0, weight=1)  # configure grid of individual tabs
optionmenu_1 = customtkinter.CTkOptionMenu(tabview.tab("PID Controller"),values=["Kp", "Ki", "Kd", "Setpoint"], dynamic_resizing=False,)
optionmenu_1.grid(row=0, column=0, padx=20, pady=(20, 10))
string_input_button = customtkinter.CTkButton(tabview.tab("PID Controller"), text="Enter the value",
                                                   command=open_input_dialog_event)
string_input_button.grid(row=2, column=0, padx=20, pady=(10, 10))
PID_Enable = customtkinter.CTkSwitch(master=tabview.tab("PID Controller"), text=f"Enable PID-controller", command = Enable_PID_controller)
PID_Enable.grid(row=3, column=0, padx=10, pady=(0, 20))
# create radiobutton frame
radio_var = tkinter.IntVar(value=0)
measuring = customtkinter.CTkTabview(root, width=250)
measuring.grid(row=0, column=3, padx=(20, 0), pady=(20, 0), sticky="nsew")
measuring.add("Motor")
measuring.add("Generator")
measuring.tab("Motor").grid_columnconfigure(0, weight=1)  # configure grid of individual tabs
measuring.tab("Generator").grid_columnconfigure(0, weight=1)
optionmenu_Measu1 = customtkinter.CTkOptionMenu(measuring.tab("Motor"),values=["Speed(RPM)", "Current", "Voltage"], dynamic_resizing=False,
                                                   )
optionmenu_Measu1.grid(row=0, column=0, padx=20, pady=(20, 10))
optionmenu_Measu2 = customtkinter.CTkOptionMenu(measuring.tab("Generator"), dynamic_resizing=False,
                                                values=["Speed(RPM)", "Current", "Voltage"])
optionmenu_Measu2.grid(row=0, column=0, padx=20, pady=(20, 10))
start1_button = customtkinter.CTkButton(measuring.tab("Motor"),text= "Start Measuring", command=Start_measuring)
start1_button.grid(row=1, column=0, padx=20, pady=10, sticky="n")
stop1_button = customtkinter.CTkButton(measuring.tab("Motor"),text= "Stop Measuring", command=Stop_measuring)
stop1_button.grid(row=2, column=0, padx=20, pady=10, sticky="n")
start2_button = customtkinter.CTkButton(measuring.tab("Generator"),text= "Start Measuring", command=Start_measuring)
start2_button.grid(row=1, column=0, padx=20, pady=10, sticky="n")
stop2_button = customtkinter.CTkButton(measuring.tab("Generator"),text= "Stop Measuring", command=Stop_measuring)
stop2_button.grid(row=2, column=0, padx=20, pady=10, sticky="n")
# create slider and progressbar frame
slider_progressbar_frame = customtkinter.CTkFrame(root, fg_color="transparent")
slider_progressbar_frame.grid(row=1, column=1, padx=(20, 0), pady=(20, 0), sticky="nsew")
slider_progressbar_frame.grid_columnconfigure(0, weight=1)
slider_progressbar_frame.grid_rowconfigure(4, weight=1)
seg_button_1 = customtkinter.CTkSegmentedButton(slider_progressbar_frame)
seg_button_1.grid(row=0, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
progressbar_2 = customtkinter.CTkProgressBar(slider_progressbar_frame)
progressbar_2.grid(row=2, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
slider_1 = customtkinter.CTkSlider(slider_progressbar_frame, from_=0, to=1, number_of_steps=4, command=slider_change)
slider_1.grid(row=3, column=0, padx=(20, 10), pady=(10, 10), sticky="ew")
plot_button = customtkinter.CTkButton(slider_progressbar_frame, text= "Step Respons", command =Open_step_respons)
plot_button.grid(row=4, column= 0,  padx=(20, 10), pady=(10, 10), sticky="ew")
# create scrollable frame
scrollable_frame = customtkinter.CTkScrollableFrame(root, label_text="Settings")
scrollable_frame.grid(row=1, column=2, padx=(20, 0), pady=(20, 0), sticky="nsew")
scrollable_frame.grid_columnconfigure(0, weight=1)
scrollable_frame_switches = []
analog_in_switch = customtkinter.CTkSwitch(master=scrollable_frame, text="Analog in", command=Analog_in_status)
analog_in_switch.grid(row=0, column=0, padx=10, pady=(0, 20))
scrollable_frame_switches.append(analog_in_switch)
analog_out_switch = customtkinter.CTkSwitch(master=scrollable_frame, text="Analog out", command= Analog_out_status)
analog_out_switch.grid(row=1, column=0, padx=10, pady=(0, 20))
scrollable_frame_switches.append(analog_out_switch)
digital_in_switch = customtkinter.CTkSwitch(master=scrollable_frame, text="Digital in", command = Digital_in_status)
digital_in_switch.grid(row=2, column=0, padx=10, pady=(0, 20))
scrollable_frame_switches.append(digital_in_switch)
digital_out_switch = customtkinter.CTkSwitch(master=scrollable_frame, text="Digital out", command=Digital_out_status)
digital_out_switch.grid(row=3, column=0, padx=10, pady=(0, 20))
scrollable_frame_switches.append(digital_out_switch)
# create checkbox and switch frame
checkbox_slider_frame = customtkinter.CTkFrame(root)
checkbox_slider_frame.grid(row=1, column=3, padx=(20, 20), pady=(10, 0), sticky="nsew")
check_var= tkinter.IntVar()
Fast_decay = customtkinter.CTkCheckBox(master=checkbox_slider_frame, text="Fast Decay", variable =check_var,  command=Fast_Decay_event)
Fast_decay.grid(row=4, column=0, pady=(20, 0), padx=20, sticky="n")
Slow_decay = customtkinter.CTkCheckBox(master=checkbox_slider_frame, text="Slow Decay",command=Slow_Decay_event)
Slow_decay.grid(row=5, column=0, pady=(20, 0), padx=20, sticky="n")
# set default values
Slow_decay.select()
scrollable_frame_switches[2].select()
scrollable_frame_switches[3].select()
appearance_mode_optionemenu.set("Light")
scaling_optionemenu.set("70%")
change_scaling_event("70")
optionmenu_1.set("Controller Parameters")
textbox.insert("0.0", "\n\n" + "text.\n\n" )
progressbar_2.set(0)
slider_1.set(0)
seg_button_1.configure(values=["Changing the Speed","Changing the Load"], command=Changing_mode)
seg_button_1.set("Changing the speed")
def extract_data(string, start_char, end_char):
    start_index = string.find(start_char)
    if start_index == -1:
        return None  
    end_index = string.find(end_char, start_index + 1)
    if end_index == -1:
        return None  
    return string[start_index + 1:end_index]    
def get_generator_rpm():
    textbox.delete("1.0","end")
    textbox.insert("0.0","\n\n"+ "Generator speed=")
def get_backEMF():
    textbox.delete("1.0","end")
    textbox.insert("0.0","\n\n"+ "Back EMF =")
def update_measurement():
    if flag:
        textbox.delete("1.0","end")
        if RPM_flag:
           textbox.insert("0.0",melding+"\n\nMotor speed=" + str(speed)+" (rpm)"+"\nSetpoint="+setpoint+"\nKp="+ Kp+"\nKi="+Ki+"\nKd="+Kd)
        elif Voltage_flag:
           textbox.insert("0.0",melding+"\n\nMotor Voltage=" + voltag_str+"(V)"+"\nSetpoint="+setpoint+"\nKp="+ Kp+"\nKi="+Ki+"\nKd="+Kd)
    root.after(70, update_measurement)    
def gui():
    root.mainloop()
dynamic_plotting= False        
def write_serial():
    global send_command_flag, read_serial_flag
    while True:
        time.sleep(0.01)
        if ser.in_waiting==0:
            if send_command_flag:
                for i in command: 
                    ser.write(i.encode())
                    time.sleep(0.1)
                print("command sent from write function")
                send_command_flag = False
                read_serial_flag= True
def serial_check():
    global y_data, x_data, t, setpoint, ref_data, speed, PID_flag
    global voltag_str
    pre_timestamp=0
    t=0
    counter=0
    if read_serial_flag==1:
        while True:
            time.sleep(0.01)
            if ser.in_waiting > 0 :
                data = ser.readline().decode('utf-8', errors='replace').rstrip()
                if data is not None:
                    pure_data= extract_data(data, '<', '>')
                if pure_data is not None:
                    if pure_data[0]=='R' and len(pure_data)==5:
                        match = re.search(r'\d+', pure_data)
                        if match:
                            speed = int(match.group())
                            try:
                                y_data.append(speed)
                                ref_data.append(int(float(setpoint)))
                                now=time.time()
                                x_data.append(t)
                                delta =now - pre_timestamp
                                t = t+delta
                                pre_timestamp=now         
                            except:
                                print("could not add to graph")
                        else:
                            speed= "null"
                    elif pure_data[0]=='p' :
                        match = re.search(r'\d+', pure_data)
                        if match:
                            p_int = int(match.group())
                            p_flt= float(p_int/100)
                            print("Kd changed=", p_flt)
                            P_ledd= str(p_flt)
                    elif pure_data[0]=='i' :
                        match = re.search(r'\d+', pure_data)
                        if match:
                            i_int = int(match.group())
                            i_flt= float(i_int/100)
                            print("Kd changed=", i_flt)
                            I_ledd= str(i_flt)
                    elif  pure_data[0]=='d' :
                        match = re.search(r'\d+', pure_data)
                        if match:
                            d_int = int(match.group())
                            d_flt= float(d_int/100)
                            print("Kd changed=", d_flt)
                            D_ledd= str(d_flt)
                    elif pure_data[0]=='z':
                        match = re.search(r'\d+', pure_data)
                        if match:
                            setpoint_int = int(match.group())
                            print("Setpoint changed=", setpoint_int)
                            setpoint= str(setpoint_int)
                    elif pure_data[0]=='V':
                        match = re.search(r'\d+', pure_data)
                        if match:
                            voltag_int = int(match.group())
                            voltage_flt= float(voltag_int/100)
                            voltag_str= str(voltage_flt)
                    elif pure_data[0]=='b':
                            match = re.search(r'\d+', pure_data)
                            if match:
                                backemf_int = int(match.group())
                                backemf_flt= float(backemf_int/100)
                                backemf_str= str(backemf_flt)
                    elif pure_data[0]=='C':
                        if pure_data[1]=='0':
                            PID_Enable.deselect()
                            PID_flag=False
                        elif pure_data[1]=='1':
                            PID_Enable.select()
                            PID_flag=True
                    if counter==10:
                        counter=0
                        ser.reset_input_buffer()
                    else:
                        counter+=1      
                else:
                    ser.reset_input_buffer()
                    ser.flush()
if __name__ == "__main__":
    t1 = threading.Thread(target=serial_check, daemon = True)
    t2 = threading.Thread(target=write_serial, daemon = True)
    t1.start()
    t2.start()
    gui()