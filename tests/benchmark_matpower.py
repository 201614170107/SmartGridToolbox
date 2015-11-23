import os
import subprocess

cases = ['case4gs.m',
         'case5.m',
         'case6ww.m',
         'case9.m',
         'case9Q.m',
         'case9target.m',
         'case14.m',
         'case14_shift.m',
         'case24_ieee_rts.m',
         'case30.m',
         'case30Q.m',
         'case30_all.m',
         'case30pwl.m',
         'case_ieee30.m',
         'case39.m',
         'case57.m',
         'case89pegase.m',
         'case118.m',
         'case300.m',
         'case1354pegase.m',
         'case2383wp.m',
         'case2736sp.m',
         'case2737sop.m',
         'case2746wop.m',
         'case2746wp.m',
         'case2869pegase.m',
         'case3012wp.m',
         'case3120sp.m',
         'case3375wp.m',
         'case9241pegase.m',
         'nesta_case3_lmbd.m',
         'nesta_case4_gs.m',
         'nesta_case5_pjm.m',
         'nesta_case6_c.m',
         'nesta_case6_ww.m',
         'nesta_case9_wscc.m',
         'nesta_case14_ieee.m',
         'nesta_case29_edin.m',
         'nesta_case30_as.m',
         'nesta_case30_fsr.m',
         'nesta_case30_ieee.m',
         'nesta_case39_epri.m',
         'nesta_case57_ieee.m',
         'nesta_case73_ieee_rts.m',
         'nesta_case89_pegase.m',
         'nesta_case118_ieee.m',
         'nesta_case162_ieee_dtc.m',
         'nesta_case189_edin.m',
         'nesta_case300_ieee.m',
         'nesta_case1354_pegase.m',
         'nesta_case1394sop_eir.m',
         'nesta_case1397sp_eir.m',
         'nesta_case1460wp_eir.m',
         'nesta_case2224_edin.m',
         'nesta_case2383wp_mp.m',
         'nesta_case2736sp_mp.m',
         'nesta_case2737sop_mp.m',
         'nesta_case2746wop_mp.m',
         'nesta_case2746wp_mp.m',
         'nesta_case2869_pegase.m',
         'nesta_case3012wp_mp.m',
         'nesta_case3120sp_mp.m',
         'nesta_case3375wp_mp.m',
         'nesta_case9241_pegase.m',
         'caseSLPQ.m',
         'caseSLPV.m',
         'caseSLPQPV.m',
         'transformer.m']
# cases = cases[0:5] # For debugging.

out = []

for c in cases:
    print(c)
    fullpath = os.path.abspath('matpower_test_cases/' + c)
    ps0 = subprocess.Popen(['benchmark_matpower.m', fullpath],
                           stdout=subprocess.PIPE)
    ps1 = subprocess.check_output(
        ['tail', '-1'],
        stdin=ps0.stdout).decode('utf8').split()[-3:]
    ps0.wait()
    n = int(ps1[0])
    out_mp_ok = int(ps1[1])
    out_mp_t = float(ps1[2])

    ps0 = subprocess.Popen(
        ['solve_network', 'nr_pol', fullpath, 'ps1', 'F', 'F'],
        stdout=subprocess.PIPE)
    ps1 = subprocess.check_output(
        ['tail', '-1'],
        stdin=ps0.stdout).decode('utf8').split()[-2:]
    ps0.wait()
    out_pol_ok = int(ps1[0])
    out_pol_t = float(ps1[1])

    ps0 = subprocess.Popen(
        ['solve_network', 'nr_rect', fullpath, 'ps1', 'F', 'F'],
        stdout=subprocess.PIPE)
    ps1 = subprocess.check_output(
        ['tail', '-1'],
        stdin=ps0.stdout).decode('utf8').split()[-2:]
    ps0.wait()
    out_rect_ok = int(ps1[0])
    out_rect_t = float(ps1[1])

    out.append((c, n, out_mp_ok, out_mp_t, out_pol_ok, out_pol_t, out_rect_ok,
                out_rect_t))

with open('benchmark.txt', mode='w', encoding='utf-8') as f:
    for l in out:
        print(l)
        f.write('{:24s} {:6d} {:1d} {:8.5f} {:1d} {:8.5f} {:1d} {:8.5f}\n'.format(*l))
