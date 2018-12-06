#include "gen_poly.h"

static int func_cnt = 0;

int main()
{
    gen_init(64);
    for (int i = 1; i <= 8; i++) {
        for (int j = 1; j <= 8; j++) {
            gen_append_poly(i, j);
        }
    }
    return 0;
}

void gen_init(int func_num)
{
    /* header file */
    FILE *hf = fopen("dynamic_poly.h", "w");
    fprintf(hf, "typedef double (*PolyFunc)");
    fprintf(hf, "(double a[], double x, long degree);\n");
    fprintf(hf, "extern const PolyFunc func_arr[];\n");
    fclose(hf);

    /* c file */
    FILE *cf = fopen("dynamic_poly.c", "w");
    fprintf(cf, "#include \"dynamic_poly.h\"\n");
    fprintf(cf, "const PolyFunc func_arr[] = {\n");
    for (int i = 0; i < func_num; i++) {
        fprintf(cf, "poly_%d,\n", i);
    }
    fprintf(cf, "};\n");
    fclose(cf);
}

void gen_append_poly(int split_num, int unrol_num)
{
    /* header file */
    FILE *hf = fopen("dynamic_poly.h", "a");
    fprintf(hf, "double poly_%d(double a[], double x, long degree);\n",
            func_cnt);
    fclose(hf);

    /* c file */
    FILE *cf = fopen("dynamic_poly.c", "a");
    /* function name */
    fprintf(cf, "double poly_%d(double a[], double x, long degree)", func_cnt);
    fprintf(cf, "{ //(%d, %d)\n", split_num, unrol_num);
    fprintf(cf, "long i;\n");

    /* variables used for splitting */
    fprintf(cf, "double result_0 = a[0];\n");
    for (int k = 1; k < split_num; k++) {
        fprintf(cf, "double result_%d = 0;\n", k);
    }

    /* compute power to the x */
    int pw_times = split_num * unrol_num;
    fprintf(cf, "double xpwr = x;\n");
    fprintf(cf, "double x_pow_0 = 1;\n");
    fprintf(cf, "double x_pow_1 = x;\n");
    for (int k = 2; k <= pw_times; k++) {
        fprintf(cf, "double x_pow_%d = x_pow_%d * x;\n", k, k - 1);
    }

    /* main for loop */
    fprintf(cf, "for (i = 1; i <= degree - %d; i += %d) {\n", pw_times - 1,
            pw_times);
    int idx_cnt = 0;
    for (int k = 0; k < split_num; k++) {
        fprintf(cf, "result_%d += (a[i + %d] * x_pow_%d", k, idx_cnt, idx_cnt);
        idx_cnt++;
        for (int l = 1; l < unrol_num; l++) {
            fprintf(cf, " + a[i + %d] * x_pow_%d", idx_cnt, idx_cnt);
            idx_cnt++;
        }
        fprintf(cf, ") * xpwr;\n");
    }
    fprintf(cf, "xpwr = xpwr * x_pow_%d;\n", pw_times);
    fprintf(cf, "}\n");

    /* remain element */
    fprintf(cf, "for (; i <= degree; i++) {\n");
    fprintf(cf, "result_0 += a[i] * xpwr;\n");
    fprintf(cf, "xpwr = xpwr * x;\n}\n");

    /* sum the all splitting variables */
    fprintf(cf, "return result_0");
    for (int k = 1; k < split_num; k++) {
        fprintf(cf, " + result_%d", k);
    }
    fprintf(cf, ";\n}\n");
    fclose(cf);

    func_cnt++;
}
