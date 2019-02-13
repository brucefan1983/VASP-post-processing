#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#define K_B                      8.617343e-5 // Boltzmann's constant  
#define TIME_UNIT_CONVERSION     1.018051e+1 // fs     <-> my natural unit
#define KAPPA_UNIT_CONVERSION    1.573769e+5 // W/(mK) <-> my natural unit


int N, Nt, Nd, Nc, pbc[3];
double T, dt, dt_in_ps, dt2, V, box[3], box2[3];
double *phi, *x, *y, *z, *vx, *vy, *vz, *hx, *hy, *hz;
double *hac_x, *hac_y, *hac_z, *rtc_x, *rtc_y, *rtc_z;


void apply_mic(double x12[3])
{
    for (int d = 0; d < 3; d++)
    {
        if (pbc[d] == 1)
        {
            if (x12[d] < - box2[d]) {x12[d] += box[d];} 
            else if (x12[d] > box2[d]) {x12[d] -= box[d];}
        }
    }
}


void find_heat_current
(
    double *x, double *y, double *z, double *vx, double *vy, double *vz, 
    double *x0, double *y0, double *z0, double& hx, double& hy, double& hz
)
{
    hx = hy = hz = 0;
    for (int n1 = 0; n1 < N; ++n1)
    {
        double u1[3];
        u1[0]=x[n1] - x0[n1];
        u1[1]=y[n1] - y0[n1];
        u1[2]=z[n1] - z0[n1];
        apply_mic(u1);
        for (int n2 = 0; n2 < N; ++n2)
        {
            double v2[3];
            v2[0] = vx[n2];
            v2[1] = vy[n2];
            v2[2] = vz[n2];
            double r12[3];
            r12[0] = x0[n2] - x0[n1];
            r12[1] = y0[n2] - y0[n1];
            r12[2] = z0[n2] - z0[n1];
            apply_mic(r12);
            double phi_u_v = 0.0;
            for (int a = 0; a < 3; a++)
            {
                int n12 = (n1 * N + n2) * 9 + a * 3;
                for (int b = 0; b < 3; b++)
                {
                    phi_u_v += phi[n12 + b] * u1[a] * v2[b];
                }
            }
            hx -= 0.5 * phi_u_v * r12[0];
            hy -= 0.5 * phi_u_v * r12[1];
            hz -= 0.5 * phi_u_v * r12[2];
        }
    }
}


void find_heat_current(void)
{
    int skip = Nt - Nd - 1;
    for (int step = 0; step < Nd; ++step)
    {
        int offset = (skip + step) * N;
        find_heat_current(x+offset, y+offset, z+offset, vx+offset, vy+offset,
            vz+offset, x, y, z, hx[step], hy[step], hz[step]);
    }
    printf("Finished calculating heat current.\n");
}


void find_hac(void)
{
    for (int nc = 0; nc < Nc; nc++)
    {
        hac_x[nc] = hac_y[nc] = hac_z[nc] = 0.0;
        int M = Nd - nc;
        for (int m = 0; m < M; m++)
        {
            hac_x[nc] += hx[m] * hx[m + nc]; 
            hac_y[nc] += hy[m] * hy[m + nc]; 
            hac_z[nc] += hz[m] * hz[m + nc]; 
        }
        hac_x[nc] /= M; hac_y[nc] /= M; hac_z[nc] /= M;
    }
    printf("Finished calculating HAC.\n");
}


void find_kappa(void)
{
    double factor = dt * 0.5 * KAPPA_UNIT_CONVERSION / (K_B * T * T * V);
    for (int nc = 0; nc < Nc; nc++) {rtc_x[nc] = rtc_y[nc] = rtc_z[nc] = 0.0;}
    for (int nc = 1; nc < Nc; nc++)
    {
        rtc_x[nc] = rtc_x[nc - 1] + (hac_x[nc - 1] + hac_x[nc]) * factor;
        rtc_y[nc] = rtc_y[nc - 1] + (hac_y[nc - 1] + hac_y[nc]) * factor;
        rtc_z[nc] = rtc_z[nc - 1] + (hac_z[nc - 1] + hac_z[nc]) * factor;
    }
    FILE *fid = fopen("hac.txt", "w");
    for (int nc = 0; nc < Nc; nc++) 
    {
        fprintf(fid, "%g %g %g %g %g %g %g\n", nc * dt_in_ps, hac_x[nc],
            hac_y[nc], hac_z[nc], rtc_x[nc], rtc_y[nc], rtc_z[nc]);
    }
    fclose(fid);
    printf("Finished calculating kappa.\n");
}


void read_phi(void)
{
    FILE *fid_phi = fopen("phi.txt", "r");
    for (int n1 = 0; n1 < N; n1++)
    {
        for (int n2 = 0; n2 < N; n2++)
        {
            int n12 = n1 * N * 9 + n2 * 9;
            for (int m = 0; m < 9; m++)
            {
                int count=fscanf(fid_phi, "%lf", &phi[n12 + m]);
                if (count != 1) 
                { printf("Reding error for phi.txt.\n"); exit(1); }
            }
        }
    }
    fclose(fid_phi);
    printf("Finished reading phi.txt.\n");
}


void read_r(void)
{
    FILE* fid_r = fopen("xf.txt", "r");
    for (int step = 0; step < Nt; ++step)
    {
        int offset = step * N;
        for (int n = 0; n < N; n++)
        {
            double fx, fy, fz; // forces are not used
            int index = offset + n;
            int count = fscanf(fid_r, "%lf%lf%lf%lf%lf%lf\n", 
                &x[index], &y[index], &z[index], &fx, &fy, &fz);
            if (count != 6) { printf("Reding error in xf.txt.\n"); exit(1); }
        }
    }
    fclose(fid_r);
    printf("Finished reading xf.txt.\n");
}


void find_v(void)
{
    for (int step = 0; step < Nt; ++step)
    {
        int offset = step * N;
        for (int n = 0; n < N; n++)
        {
            int index = offset + n;
            if (step > 0 && step < Nt-1)
            {
                double r12[3];
                int index_m = index - N;
                int index_p = index + N;
                r12[0] = x[index_p] - x[index_m];
                r12[1] = y[index_p] - y[index_m];
                r12[2] = z[index_p] - z[index_m];
                apply_mic(r12);
                vx[index] = r12[0] / dt2;
                vy[index] = r12[1] / dt2;
                vz[index] = r12[2] / dt2;
            }
        }
    }
    printf("Finished calculating velocity.\n");
}


void read_para(void)
{
    FILE *fid_para = fopen("para.txt", "r");
    int count = fscanf(fid_para, "%d%d%d%d", &N, &Nt, &Nd, &Nc);
    if (count != 4) { printf("Reding error for para.in.\n"); exit(1); }
    count = fscanf(fid_para, "%d%d%d", &pbc[0], &pbc[1], &pbc[2]);
    if (count != 3) { printf("Reding error for para.in.\n"); exit(1); }
    count = fscanf(fid_para, "%lf%lf%lf", &box[0], &box[1], &box[2]);
    if (count != 3) { printf("Reding error for para.in.\n"); exit(1); }
    count = fscanf(fid_para, "%lf%lf", &T, &dt);
    if (count != 2) { printf("Reding error for para.in.\n"); exit(1); }
    fclose(fid_para);
}


void find_para(void)
{
    dt_in_ps = dt / 1000.0;
    dt /= TIME_UNIT_CONVERSION;
    dt2 = dt * 2.0;
    for (int d = 0; d < 3; d++) box2[d] = box[d] * 0.5;
    V = box[0] * box[1] * box[2];
}


void echo_para()
{
    printf("N = %d\n", N);
    printf("Nt = %d\n", Nt);
    printf("Nd = %d\n", Nd);
    printf("Nc = %d\n", Nc);
    printf("pbc = %d %d %d\n", pbc[0], pbc[1], pbc[2]);
    printf("box = %g %g %g (A)\n", box[0], box[1], box[2]);
    printf("T = %g (K)\n", T);
    printf("dt = %g (fs)\n", dt);
}


void allocate_memory(void)
{
    phi = (double*) malloc(N*N*9*sizeof(double));
    x  = (double*) malloc(N*Nt * sizeof(double));
    y  = (double*) malloc(N*Nt * sizeof(double));
    z  = (double*) malloc(N*Nt * sizeof(double));
    vx = (double*) malloc(N*Nt * sizeof(double));
    vy = (double*) malloc(N*Nt * sizeof(double));
    vz = (double*) malloc(N*Nt * sizeof(double));
    hx = (double*) malloc(Nd * sizeof(double));
    hy = (double*) malloc(Nd * sizeof(double));
    hz = (double*) malloc(Nd * sizeof(double));
    hac_x = (double *)malloc(sizeof(double) * Nc);
    hac_y = (double *)malloc(sizeof(double) * Nc);
    hac_z = (double *)malloc(sizeof(double) * Nc);
    rtc_x = (double *)malloc(sizeof(double) * Nc);
    rtc_y = (double *)malloc(sizeof(double) * Nc);
    rtc_z = (double *)malloc(sizeof(double) * Nc);
    printf("Memory allocated.\n");
}


void free_memory(void)
{ 
    free(phi); free(x); free(y); free(z); free(vx); free(vy); free(vz);
    free(hx); free(hy); free(hz);
    free(hac_x); free(hac_y); free(hac_z);
    free(rtc_x); free(rtc_y); free(rtc_z);
    printf("Memory freed.\n");
}


int main(int argc, char *argv[])
{
    read_para();
    echo_para();
    find_para();
    allocate_memory();
    read_phi();
    read_r();
    find_v();
    find_heat_current();
    find_hac();
    find_kappa();
    free_memory();
    printf("Done.\n");
    return 0;
}


