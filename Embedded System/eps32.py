from machine import Pin, PWM
from time import sleep_us, sleep


rx = Pin(16, Pin.IN)
tx = Pin(17, Pin.OUT)
tx.on()

#address = 12 -> 0000 1100
led1 = PWM(Pin(12), freq=2000)
led1.duty(0)
#address = 14 -> 0000 1110
led2 = PWM(Pin(14), freq=2000)
led2.duty(0)
#address = 27 -> 0001 1011
led3 = PWM(Pin(27), freq=2000)
led3.duty(0)
#address = 26 -> 0001 1010
led4 = PWM(Pin(26), freq=2000)
led4.duty(0)
#address = 25 -> 0001 1001
led5 = PWM(Pin(25), freq=2000)
led5.duty(0)
#address = 15 -> 0000 1111
led6 = PWM(Pin(15), freq=2000)
led6.duty(0)

addresses = ["00001100", "00001110", "00011011", "00011010", "00011001", "00001111"]

signal = [1] * 38
collecting = False
index = 1
length = 1
result = ""

def map_lightness(data):
    # 8 bits
    int_value = 0
    for idx, value in enumerate(data):
        if value == "1":
            int_value += 2**(len(data) - 1 - idx)    
    in_min = 0
    in_max = 255
    out_min = 0
    out_max = 1023
    
    return int((int_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)


print("ESP32 is up and running")


while True:
    value = rx.value()
    if value == 0 and length == 1:
        collecting = True
        print('Collecting....')
    
    if collecting:
        signal[index] = value
        index += 1
        length += 1
        
        #sleep(time_)
        sleep_us(10_000)
        
    if length == 38:
        index = 1
        length = 1
        collecting = False
        
        for i in range(0, 38, 2):
            print(signal[i], end=" ")
            result += str(signal[i])
            
        signal = [1] * 38
        
        address = result[1:9]
        data = result[9:17]
        print('\ndata:', data)
        result = ""
        
        try:
            led_idx = addresses.index(address)
        except Exception as e:
            led_idx = -1
        
        if led_idx == 0:
            print('led1')
            duty_val = map_lightness(data)
            led1.duty(duty_val)
        elif led_idx == 1:
            print('led2')
            duty_val = map_lightness(data)
            led2.duty(duty_val)
        elif led_idx == 2:
            print('led3')
            duty_val = map_lightness(data)
            led3.duty(duty_val)
        elif led_idx == 3:
            print('led4')
            duty_val = map_lightness(data)
            led4.duty(duty_val)
        elif led_idx == 4:
            print('led5')
            duty_val = map_lightness(data)
            led5.duty(duty_val)
        elif led_idx == 5:
            print('led6')
            duty_val = map_lightness(data)
            led6.duty(duty_val)
        else:
            print("There is no such led")
        
        
        
        
        
