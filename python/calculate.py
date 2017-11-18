import math

a = 80000.0
last_val = 200

def calc_s(tm):
	t = 0.001 * tm
	s = round(0.5 * a * t * t)
	return s

def calc_p(s):
	t0 = math.sqrt(2.0*s/a)
	t1 = math.sqrt(2.0*(s+1)/a)
	dt = round((t1 - t0) * 12000000)
	return dt
	

def get_array(start_time, last_val):
	arr = []
	last_period = 9999
	last_scaler = 9999
	tm = start_time - 1
	i = 0
	while (last_period>last_val) or (last_scaler>0):
		tm += 1
		s = calc_s(tm)
		p = calc_p(s)
		if p > 1023:
			scaler = 2
			period = round(p / 16) - 1
			true_p = 16 * (period+1)
		elif p > 256:
			scaler = 1
			period = round(p / 4) - 1
			true_p = 4 * (period+1)
		else:
			scaler = 0
			period = p - 1
			true_p = period + 1
		if (period==last_period) and (scaler==last_scaler):
			continue
		f = 12000000.0 / true_p
		spd = (36000.0 * f)/ (200.0 * 16.0 * 2.0 * 90.0)
		arr.append((i, tm, s, scaler, period, spd))
		last_period = period
		last_scaler = scaler
		i += 1
	return arr

def get_start():
	arr = []
	s = 0
	t = 0
	while True:
		dt = calc_p(s)
		if dt <= 256*16:
			break
		f = 12000000.0 /dt
		spd = (36000.0 * f)/ (200.0 * 16.0 * 2.0 * 90.0)
		arr.append((s, round(0.5*dt/128.0), t/12000, spd))
		t += dt
		s += 1
	return t/12000, arr

def write_file(start_data, data):
    start_length = len(start_data)
    length = len(data)
    with open('..\motor_config.h', 'w') as f:
                
        #Starting values
        f.write('const uint8_t motor_start_lookup[{}] =\n'.format(start_length))
        f.write('{\n')
        for rec in start_data:
            i = rec[0]
            t = rec[2]
            y = rec[1]
            y = min([y, 0xFFFF])
            if i == length-1:
                    line = '  {0} /*i={1}, t={2}*/\n'
            else:
                    line = '  {0}, /*i={1}, t={2}*/\n'
            line = line.format(y, i, t)
            f.write(line)
        f.write('};\n\n')
		
        #Steps
        f.write('const uint16_t motor_step_lookup[{}] =\n'.format(length))
        f.write('{\n')
        for rec in data:
                i = rec[0]
                t = rec[1]
                s = rec[2]
                if i == length-1:
                        line = '  {0} /*i={1}, t={2}*/\n'
                else:
                        line = '  {0}, /*i={1}, t={2}*/\n'
                line = line.format(s, i, t)
                f.write(line)
        f.write('};\n\n')
        
        #Prescaler
        f.write('const uint8_t motor_prescaler_lookup[{}] =\n'.format(length))
        f.write('{\n')
        for rec in data:
                i = rec[0]
                t = rec[1]
                x = rec[3]
                if i == length-1:
                        line = '  {0} /*i={1}, t={2}*/\n'
                else:
                        line = '  {0}, /*i={1}, t={2}*/\n'
                line = line.format(x, i, t)
                f.write(line)
        f.write('};\n\n')
        
        #Period
        f.write('const uint8_t motor_period_lookup[{}] =\n'.format(length))
        f.write('{\n')
        for rec in data:
                i = rec[0]
                t = rec[1]
                p = rec[4]
                if i == length-1:
                        line = '  {0} /*i={1}, t={2}*/\n'
                else:
                        line = '  {0}, /*i={1}, t={2}*/\n'
                line = line.format(p, i, t)
                f.write(line)
        f.write('};\n\n')
        
        #Speed
        f.write('const uint16_t motor_speed_lookup[{}] =\n'.format(start_length + length))
        f.write('{\n')
        for rec in start_data:
                i = rec[0]
                spd = round(rec[3])
                if i == length-1:
                        line = '  {0} /*i={1}*/\n'
                else:
                        line = '  {0}, /*i={1}*/\n'
                line = line.format(spd, i)
                f.write(line)
        for rec in data:
                i = rec[0]
                spd = round(rec[5])
                if i == length-1:
                        line = '  {0} /*i={1}, j={2}*/\n'
                else:
                        line = '  {0}, /*i={1}, j={2}*/\n'
                line = line.format(spd, start_length+i, i)
                f.write(line)
        f.write('};\n')

start_time, start_data = get_start()
data = get_array(start_time, last_val)
write_file(start_data, data)

for rec in start_data:
	print(rec)
for rec in data[:5]:
	print(rec)
