/*--------------------------------------------------------------------
This source distribution is placed in the public domain by its author,
Jason Papadopoulos. You may use it for any purpose, free of charge,
without having to notify anyone. I disclaim any responsibility for any
errors.

Optionally, please be nice and tell me if you find this source to be
useful. Again optionally, if you add to the functionality present here
please consider making those additions public too, so that others may 
benefit from your work.	

$Id$
--------------------------------------------------------------------*/

#include "poly_skew.h"

#if MAX_POLY_DEGREE < 6
#error "Polynomial generation assumes degree <= 6 allowed"
#endif

typedef struct {
	double digits;
	double stage1_norm;
	double stage2_norm;
	double final_norm;
} poly_param_t;

typedef struct {
	FILE *all_poly_file;
	poly_config_t *config;
} stage2_callback_data_t;

static const poly_param_t prebuilt_params_deg4[] = {

	/* determined by experiment */

	{ 80, 3.00E+013, 2.00E+013, 1.00E-007},
	{ 85, 3.00E+014, 4.00E+013, 6.50E-008},
	{ 90, 2.50E+015, 5.00E+014, 3.80E-008},
	{ 95, 1.50E+016, 1.00E+015, 1.50E-008},
	{100, 2.10E+017, 4.00E+015, 8.30E-009},
	{105, 1.00E+018, 1.50E+016, 3.00E-009},
	{110, 6.00E+018, 5.00E+016, 1.00E-009},
};

static const poly_param_t prebuilt_params_deg5[] = {

	/* shamelessly stolen from GGNFS */

	{100, 5.56E+014, 2.86E+013, 2.80E-009},
	{101, 8.13E+014, 4.08E+013, 2.50E-009},
	{102, 1.19E+015, 5.82E+013, 2.21E-009},
	{103, 1.74E+015, 8.31E+013, 1.94E-009},
	{104, 2.55E+015, 1.18E+014, 1.71E-009},
	{105, 3.74E+015, 1.69E+014, 1.50E-009},
	{106, 5.47E+015, 2.41E+014, 1.32E-009},
	{107, 8.01E+015, 3.44E+014, 1.16E-009},
	{108, 1.17E+016, 4.91E+014, 1.02E-009},
	{109, 1.72E+016, 7.00E+014, 8.92E-010},
	{110, 2.52E+016, 9.98E+014, 7.83E-010},
	{111, 3.68E+016, 1.42E+015, 6.88E-010},
	{112, 5.39E+016, 2.03E+015, 6.04E-010},
	{113, 7.90E+016, 2.90E+015, 5.31E-010},
	{114, 1.16E+017, 4.13E+015, 4.66E-010},
	{115, 1.69E+017, 5.90E+015, 4.09E-010},
	{116, 2.48E+017, 8.41E+015, 3.60E-010},
	{117, 3.63E+017, 1.20E+016, 3.16E-010},
	{118, 5.31E+017, 1.71E+016, 2.77E-010},
	{119, 7.78E+017, 2.44E+016, 2.44E-010},
	{120, 1.14E+018, 3.48E+016, 2.14E-010},
	{121, 1.67E+018, 4.97E+016, 1.88E-010},
	{122, 2.44E+018, 7.09E+016, 1.65E-010},
	{123, 3.58E+018, 1.01E+017, 1.45E-010},
	{124, 5.24E+018, 1.44E+017, 1.27E-010},
	{125, 7.67E+018, 2.06E+017, 1.12E-010},
	{126, 1.12E+019, 2.94E+017, 9.82E-011},
	{127, 1.64E+019, 4.19E+017, 8.62E-011},
	{128, 2.41E+019, 5.98E+017, 7.57E-011},
	{129, 3.52E+019, 8.52E+017, 6.65E-011},
	{130, 5.16E+019, 1.22E+018, 5.84E-011},
	{131, 7.55E+019, 1.73E+018, 5.13E-011},
	{132, 1.11E+020, 2.47E+018, 4.51E-011},
	{133, 1.62E+020, 3.53E+018, 3.96E-011},
	{134, 2.37E+020, 5.04E+018, 3.48E-011},
	{135, 3.47E+020, 7.18E+018, 3.05E-011},
	{136, 5.08E+020, 1.02E+019, 2.68E-011},
	{137, 7.44E+020, 1.46E+019, 2.36E-011},
	{138, 1.09E+021, 2.09E+019, 2.07E-011},
	{139, 1.60E+021, 2.98E+019, 1.82E-011},
	{140, 2.34E+021, 4.24E+019, 1.60E-011},
	{141, 3.42E+021, 6.05E+019, 1.40E-011},
	{142, 5.01E+021, 8.64E+019, 1.23E-011},
	{143, 7.33E+021, 1.23E+020, 1.08E-011},
	{144, 1.07E+022, 1.76E+020, 9.50E-012},
	{145, 1.57E+022, 2.51E+020, 8.34E-012},
	{146, 2.30E+022, 3.58E+020, 7.33E-012},
	{147, 3.37E+022, 5.10E+020, 6.43E-012},
	{148, 4.94E+022, 7.28E+020, 5.65E-012},
	{149, 7.23E+022, 1.04E+021, 4.96E-012},
	{150, 1.06E+023, 1.48E+021, 4.36E-012},
	{151, 1.55E+023, 2.11E+021, 3.83E-012},
	{152, 2.27E+023, 3.01E+021, 3.36E-012},
	{153, 3.32E+023, 4.30E+021, 2.95E-012},
	{154, 8.00E+023, 4.90E+021, 2.40E-012},
	{155, 1.00E+024, 6.80E+021, 2.00E-012},

	/* contributed by Tom Womack */

	{159, 2.00E+024, 2.00E+022, 1.00E-012},
	{165, 8.00E+024, 2.00E+023, 5.00E-013},
	{170, 5.00E+025, 1.58E+024, 1.50E-013},

	/* contributed by Serge Batalov */

	{175, 3.00E+026, 1.00E+025, 1.00E-013},
	{180, 1.80E+027, 5.36E+025, 7.00E-014},
	{185, 1.00E+028, 3.12E+026, 2.00E-014},
	{190, 6.00E+028, 1.82E+027, 4.00E-015},

	/* from Tom Womack */

	{197, 1.00E+030, 1.00E+029, 2.00E-015},

	/* only a guess */

	{200, 3.10E+030, 1.10E+029, 1.50E-015},
	{205, 2.00E+031, 5.70E+029, 5.50E-016},
	{210, 1.00E+032, 3.00E+030, 1.90E-016},
	{215, 6.00E+032, 1.50E+031, 7.00E-017},
	{220, 2.40E+033, 7.70E+031, 3.00E-017},
};

static const poly_param_t prebuilt_params_deg6[] = {

	{140, 2.34E+017, 5.00E+018, 1.7e-012},
	{141, 2.34E+017, 5.00E+018, 1.7e-012},

	/* contributed by Paul Leyland */

 	{200, 1.00E+026, 1.00E+025, 8.0e-018},
 	{205, 1.00E+027, 1.00E+026, 6.0e-019},
 	{230, 3.00E+029, 3.00E+029, 6.0e-019},
 	{235, 1.50E+030, 8.00E+029, 6.0e-019},

	/* better than nothing (marginally) */

 	{305, 1.00E+040, 1.00E+041, 0},
 	{310, 1.00E+042, 1.00E+043, 0},
};

/*--------------------------------------------------------------------*/
static double get_bernstein_mult(double digits, uint32 degree) {

	/* the conversion from murphy to bernstein score
	   approximately obeys a simple power law */

	switch (degree) {
	case 4: return 30000.0 * pow(2.5, (digits - 90) / 5);
	case 5:	return 2.2 * pow(3.0, (digits - 100) / 10);
	case 6:	return 0.012 * pow(3.5, (digits - 140) / 15);  /* BROKEN! */
	}

	return 0;
}

/*--------------------------------------------------------------------*/
static void get_poly_params(double digits, poly_param_t *params,
				const poly_param_t *defaults, 
				uint32 num_default_entries) {

	uint32 i;
	const poly_param_t *low, *high;
	double j, k, dist;
	double max_digits;

	/* if the input is too small (large), just use 
	   the first (last) table entry */

	if (digits < defaults[0].digits) {
		*params = defaults[0];
		return;
	}

	max_digits = defaults[num_default_entries - 1].digits;
	if (digits >= max_digits) {
		if (digits > max_digits + 5) {
			printf("error: no parameters for "
				"%.0lf digit inputs\n", digits + 0.5);
			exit(-1);
		}
		*params = defaults[num_default_entries - 1];
		return;
	}

	/* Otherwise the parameters to use are a weighted average 
	   of the two table entries the input falls between */

	for (i = 0; i < num_default_entries - 1; i++) {
		if (digits < defaults[i+1].digits)
			break;
	}

	low = &defaults[i];
	high = &defaults[i+1];
	dist = high->digits - low->digits;
	j = digits - low->digits;
	k = high->digits - digits;

	/* use exponential interpolation */

	params->digits = digits;
	params->stage1_norm = exp((log(low->stage1_norm) * k +
			           log(high->stage1_norm) * j) / dist);
	params->stage2_norm = exp((log(low->stage2_norm) * k +
			           log(high->stage2_norm) * j) / dist);
	params->final_norm = exp((log(low->final_norm) * k +
			           log(high->final_norm) * j) / dist);
}

/*------------------------------------------------------------------*/
static void stage1_callback(mpz_t high_coeff, mpz_t p, mpz_t m, 
				double coeff_bound, void *extra) {
	
	gmp_printf("%Zd %Zd %Zd\n", high_coeff, p, m);
	poly_stage2_run((poly_stage2_t *)extra, high_coeff, p, m, 
			coeff_bound, NULL);
}

/*------------------------------------------------------------------*/
static void stage1_callback_log(mpz_t high_coeff, mpz_t p, mpz_t m, 
				double coeff_bound, void *extra) {
	
	FILE *mfile = (FILE *)extra;
	gmp_printf("%Zd %Zd %Zd\n", high_coeff, p, m);
	gmp_fprintf(mfile, "%Zd %Zd %Zd\n",
			high_coeff, p, m);
	fflush(mfile);
}

/*------------------------------------------------------------------*/
static void
stage2_callback(void *extra, uint32 degree, 
		mpz_t * coeff1, mpz_t * coeff2,
		double skewness, double size_score,
		double root_score, double combined_score,
		uint32 num_real_roots)
{
	uint32 i;
	poly_select_t poly;
	mpz_poly_t *rpoly;
	mpz_poly_t *apoly;
	stage2_callback_data_t *data = (stage2_callback_data_t *)extra;
	poly_config_t *config = data->config;

	memset(&poly, 0, sizeof(poly_select_t));
	rpoly = &poly.rpoly;
	apoly = &poly.apoly;
	mpz_poly_init(rpoly);
	mpz_poly_init(apoly);

	rpoly->degree = 1;
	for (i = 0; i <= 1; i++)
		mpz_set(rpoly->coeff[i], coeff2[i]);

	apoly->degree = degree;
	for (i = 0; i <= degree; i++)
		mpz_set(apoly->coeff[i], coeff1[i]);

	poly.root_score = root_score;
	poly.size_score = size_score;
	poly.combined_score = combined_score;
	poly.skewness = skewness;

	printf("save %le %.4lf %.2lf %le rroots %u\n", size_score,
			root_score, skewness, combined_score,
			num_real_roots);

	fprintf(data->all_poly_file, 
		"# norm %le alpha %lf e %.3le rroots %u\nskew: %.2lf\n", 
		size_score, root_score, combined_score, 
		num_real_roots, skewness);
	for (i = 0; i <= degree; i++)
		gmp_fprintf(data->all_poly_file, "c%u: %Zd\n", i, coeff1[i]);
	for (i = 0; i <= 1; i++)
		gmp_fprintf(data->all_poly_file, "Y%u: %Zd\n", i, coeff2[i]);
	fflush(data->all_poly_file);

	save_poly(config, &poly);
	mpz_poly_free(rpoly);
	mpz_poly_free(apoly);
}

/*------------------------------------------------------------------*/
static void find_poly_core(msieve_obj *obj, mpz_t n,
			poly_config_t *config,
			uint32 degree, uint32 deadline) {

	poly_stage1_t stage1_data;
	poly_stage2_t stage2_data;
	stage2_callback_data_t stage2_callback_data;
	poly_param_t params;
	char buf[2048];
	FILE *stage1_outfile = NULL;
	uint32 do_both_stages = 0;
	double digits;
	double coeff_scale = 3.0;

	if ((obj->flags & MSIEVE_FLAG_NFS_POLY1) &&
	    (obj->flags & MSIEVE_FLAG_NFS_POLY2))
		do_both_stages = 1;

	/* get poly select parameters */

	digits = log(mpz_get_d(n)) / log(10.0);

	switch (degree) {
	case 4:
		get_poly_params(digits, &params, prebuilt_params_deg4,
				sizeof(prebuilt_params_deg4) / 
					sizeof(poly_param_t));
		break;

	case 5:
		get_poly_params(digits, &params, prebuilt_params_deg5,
				sizeof(prebuilt_params_deg5) / 
					sizeof(poly_param_t));
		break;

	case 6:
		coeff_scale = 1;
		get_poly_params(digits, &params, prebuilt_params_deg6,
				sizeof(prebuilt_params_deg6) / 
					sizeof(poly_param_t));
		break;

	default:
		printf("error: invalid degree %u\n", degree);
		exit(-1);
	}

	/* set up stage 1 */

	if (obj->flags & MSIEVE_FLAG_NFS_POLY1) {

		/* if we're doing both stage 1 and 2, every hit
		   in stage 1 is immediately forwarded to stage 2.
		   For stage 1 alone, all the stage 1 hits are buffered
		   to file first */

		if (do_both_stages) {
			poly_stage1_init(&stage1_data, 
					stage1_callback, &stage2_data);
		}
		else {
			sprintf(buf, "%s.m", obj->savefile.name);
			stage1_outfile = fopen(buf, "a");
			if (stage1_outfile == NULL) {
				printf("error: cannot open poly1 file\n");
				exit(-1);
			}
			poly_stage1_init(&stage1_data, stage1_callback_log, 
					stage1_outfile);
		}

		/* fill stage 1 data */

		mpz_set(stage1_data.gmp_N, n);
		stage1_data.degree = degree;
		stage1_data.norm_max = params.stage1_norm;
		stage1_data.deadline = deadline;

		if (obj->nfs_lower)
			uint64_2gmp(obj->nfs_lower, 
					stage1_data.gmp_high_coeff_begin);
		else
			mpz_set_ui(stage1_data.gmp_high_coeff_begin, 
					(unsigned long)1);

		if (obj->nfs_upper)
			uint64_2gmp(obj->nfs_upper, 
					stage1_data.gmp_high_coeff_end);
		else
			mpz_set_d(stage1_data.gmp_high_coeff_end,
				pow(mpz_get_d(stage1_data.gmp_N), 1.0 / 
					(double)(degree * (degree - 1))) / 
				coeff_scale );

		if (obj->nfs_lower && obj->nfs_upper)
			stage1_data.deadline = 0;
		else
			logprintf(obj, "time limit set to %.2f CPU-hours\n",
				stage1_data.deadline / 3600.0);

		{ /* SB: tried L[1/3,c] fit; it is no better than this */
			double e0 = (digits >= 121) ? (0.0607 * digits + 2.25):
				                      (0.0526 * digits + 3.23);
			if (degree == 4) e0 = 0.0625 * digits + 1.69;
			e0 = exp(-log(10) * e0); 
#ifdef HAVE_CUDA
			e0 *= 1.15;
#endif
			logprintf(obj, "expecting poly E from %.2le to > %.2le\n",
				e0, 1.15 * e0);
			/* seen exceptional polys with +40% but that's */
			/* very rare. The fit is good for 88..232 digits */
		}
 
		logprintf(obj, "searching leading coefficients from "
				"%.0lf to %.0lf\n",
				mpz_get_d(stage1_data.gmp_high_coeff_begin),
				mpz_get_d(stage1_data.gmp_high_coeff_end));
	}

	/* set up stage 2 */

	if (obj->flags & MSIEVE_FLAG_NFS_POLY2) {

		poly_stage2_init(&stage2_data, obj, stage2_callback, 
				&stage2_callback_data);

		/* fill stage 2 data */

		mpz_set(stage2_data.gmp_N, n);
		stage2_data.degree = degree;
		stage2_data.max_norm = params.stage2_norm;
		stage2_data.min_e = params.final_norm;
		stage2_data.min_e_bernstein = params.final_norm / 
					get_bernstein_mult(digits, degree);

		sprintf(buf, "%s.p", obj->savefile.name);
		stage2_callback_data.config = config;
		stage2_callback_data.all_poly_file = fopen(buf, "a");
		if (stage2_callback_data.all_poly_file == NULL) {
			printf("error: cannot open poly2 file\n");
			exit(-1);
		}
	}


	if (do_both_stages) {
		poly_stage1_run(obj, &stage1_data);

		fclose(stage2_callback_data.all_poly_file);
		poly_stage1_free(&stage1_data);
		poly_stage2_free(&stage2_data);
	}
	else if (obj->flags & MSIEVE_FLAG_NFS_POLY1) {
		poly_stage1_run(obj, &stage1_data);
		fclose(stage1_outfile);
		poly_stage1_free(&stage1_data);
	}
	else if (obj->flags & MSIEVE_FLAG_NFS_POLY2) {
		uint32 i;
		mpz_t ad, p, m;
		mpz_t full_apoly[MAX_POLY_DEGREE + 1];

		mpz_init(ad);
		mpz_init(p);
		mpz_init(m);
		for (i = 0; i <= MAX_POLY_DEGREE; i++)
			mpz_init(full_apoly[i]);

		sprintf(buf, "%s.m", obj->savefile.name);
		stage1_outfile = fopen(buf, "r");
		if (stage1_outfile == NULL) {
			printf("error: cannot open poly2 input file\n");
			exit(-1);
		}

		while (1) {
			int c;
			char *tmp = buf;
			mpz_t *arg = NULL;

			if (fgets(buf, sizeof(buf), stage1_outfile) == NULL)
				break;

			if (gmp_sscanf(tmp, "%Zd%n", ad, &c) != 1)
				continue;
			tmp += c;

			if (mpz_cmp_ui(ad, 0) == 0) {
				for (i = 0; i <= degree; i++) {
					if (gmp_sscanf(tmp, "%Zd%n", 
					    full_apoly[degree - i], &c) != 1)
						break;
					tmp += c;
				}

				mpz_set(ad, full_apoly[degree]);
				arg = full_apoly;
			}
			if (gmp_sscanf(tmp, "%Zd %Zd", p, m) != 2)
				continue;

			if (arg == NULL)
				gmp_printf("poly %Zd %Zd %Zd\n", ad, p, m);

			poly_stage2_run(&stage2_data, ad, p, m, 1e100, arg);

			if (obj->flags & MSIEVE_FLAG_STOP_SIEVING)
				break;
		}

		mpz_clear(ad);
		mpz_clear(p);
		mpz_clear(m);
		for (i = 0; i <= MAX_POLY_DEGREE; i++)
			mpz_clear(full_apoly[i]);
		fclose(stage1_outfile);
		fclose(stage2_callback_data.all_poly_file);
		poly_stage2_free(&stage2_data);
	}
}

/*------------------------------------------------------------------*/
void find_poly_skew(msieve_obj *obj, mpz_t n,
			poly_config_t *config,
			uint32 deadline) {

	/* search for NFS polynomials */

	uint32 bits = mpz_sizeinbase(n, 2);

	if (bits < 363) {		/* <= 110 digits */
		find_poly_core(obj, n, config, 4, deadline);
	}
	else if (bits < 726) {		/* 110-220 digits */
		find_poly_core(obj, n, config, 5, deadline);
	}
	else {				/* 220+ digits */
		find_poly_core(obj, n, config, 6, deadline);
	}
}
