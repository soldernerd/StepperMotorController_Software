import math

a = 80000.0


def t_from_s(s):
       t = math.sqrt(2.0*s/a)
       return t

def f_from_t(t):
       f = t * a
       return f

def params_from_f(f):
       div = 48000000.0 / (4.0 * f)
       pre = 1
       post = 1
       period = 256
       if round(div)<=256:
             period = round(div)
       elif round(div/4.0)<=256:
             pre = 4
             period = round(div/4.0)
       elif round(div/16.0)<=256:
             pre = 16
             period = round(div/16.0)
       else:
             pre = 16
             for post in [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]:
                    d = div / (2 * pre * post)
                    if round(d)<=256:
                           period = round(d)
                           break
       return (pre, period, post)

def v_from_params(params):
       div = params[0] * params[1] * params[2]
       if params[2] > 1:
             div *= 2
       f = 48000000.0 / (4.0 * div)
       v = (f * 36000.0) / (3200.0 * 2.0 * 90.0)
       return round(v)

def spds_from_array(arr):
       spds = [rec[4] for rec in arr]
       return spds

def stps_from_array(arr):
       stps = [rec[1] for rec in arr]
       return stps

def pre_from_array(arr):
       tmp = [rec[-1][0] for rec in arr]
       pre = []
       for val in tmp:
             if val==16:
                    pre.append(2)
             elif val==4:
                    pre.append(1)
             else:
                    pre.append(0)
       return pre

def post_from_array(arr):
       post = [rec[-1][2]-1 for rec in arr]
       return post

def per_from_array(arr):
       per = [rec[-1][1]-1 for rec in arr]
       return per
       
def calculate(max_f):
       i = 0
       s = 1
       f = 0.0
       arr = []
       while f < max_f:
             t = t_from_s(s)
             f = f_from_t(t)
             params = params_from_f(f)
             if (s==1) or ((not (params==arr[-1][-1])) and (t-arr[-1][2]>0.0005)):
                    v = v_from_params(params)
                    arr.append((i, s, t, f, v, params))
                    #print(arr[-1])
                    i += 1
             s += 1
       return arr

def list_to_string(lst):
       length = len(lst)
       txt = '{\n'
       for i in range(length):
             if i<length-1:
                    txt += '    {0}, /*i={1}*/\n'.format(lst[i], i)
             else:
                    txt += '    {0} /*i={1}*/\n'.format(lst[i], i)
       txt += '};\n\n'
       return txt

def write_file(filename):
       print('Writing file {0}'.format(filename))
       #collect and prepare data
       arr = calculate(60000)
       length = len(arr)
       for rec in arr:
             print(rec)
       stps = stps_from_array(arr)
       pre = pre_from_array(arr)
       per = per_from_array(arr)
       post = post_from_array(arr)
       spds = spds_from_array(arr)
       #write to file
       with open(filename, 'w') as f:
             f.write('const uint16_t motor_steps_lookup[{0}] =\n'.format(length))
             f.write(list_to_string(stps))
             f.write('const uint8_t motor_prescaler_lookup[{0}] =\n'.format(length))
             f.write(list_to_string(pre))
             f.write('const uint8_t motor_period_lookup[{0}] =\n'.format(length))
             f.write(list_to_string(per))
             f.write('const uint8_t motor_postscaler_lookup[{0}] =\n'.format(length))
             f.write(list_to_string(post))
             f.write('const uint16_t motor_speed_lookup[{0}] =\n'.format(length))
             f.write(list_to_string(spds))
       print('done')

write_file('..\motor_config.h')
