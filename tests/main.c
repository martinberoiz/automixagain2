#include "automix.h"
#include "float.h"
#include "logwrite.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double logp_truncnormal_sampler(int model_k, int mdim, double *xp);
double logp_normal_sampler(int model_k, int mdim, double *xp);
double logp_beta_sampler(int model_k, int mdim, double *xp);
double logp_normal_params(int model_k, int mdim, double *params);
double logp_beta_params(int model_k, int mdim, double *params);
double logp_gamma_params(int model_k, int mdim, double *params);
double logp_gamma_beta(int model_k, int mdim, double *params);
double logp_normal_beta(int model_k, int mdim, double *params);
double logp_normal_gamma(int model_k, int mdim, double *params);
void init_normal_sampler(int model_k, int mdim, double *xp);
void init_truncnormal_sampler(int model_k, int mdim, double *xp);
void init_beta_sampler(int model_k, int mdim, double *xp);
void init_normal_params(int model_k, int mdim, double *xp);
void init_beta_params(int model_k, int mdim, double *xp);
void init_gamma_params(int model_k, int mdim, double *xp);
void init_gamma_beta(int model_k, int mdim, double *xp);
void init_normal_gamma(int model_k, int mdim, double *xp);
void init_normal_beta(int model_k, int mdim, double *xp);

int test_setUp(int models, int *model_dims, targetFunc logposterior,
               rwmInitFunc initRWM);
int test_tearDown(char *filename, int nmodels);

int test_sampler(double true_mean, double true_sigma, double lower,
                 double upper);
int test_dist_params(double true_param1, double true_param2);
int test_two_models(double true_k1_p1, double true_k1_p2, double true_k2_p1,
                    double true_k2_p2, double true_k1_frac);

int nsamples = 10;
double data_samples[] = {0.50613293, 0.70961096, 0.28166951, 0.12532996,
                         0.46374168, 0.58337466, 0.52458217, 0.56052633,
                         0.57215576, 0.68698825};

int main(int argc, char *argv[]) {
  int pass = EXIT_SUCCESS;
  int model_dim;
  int model_dims[2];

  printf("Test Normal Sampler: . . .\n");
  model_dim = 1;
  test_setUp(1, &model_dim, logp_normal_sampler, init_normal_sampler);
  pass |= test_sampler(0.5, 1.0, -DBL_MAX, DBL_MAX);
  test_tearDown("test", 1);

  printf("Test Truncated Normal Sampler: . . .\n");
  model_dim = 1;
  test_setUp(1, &model_dim, logp_truncnormal_sampler, init_truncnormal_sampler);
  pass |= test_sampler(1.3, 1.5, 0.0, 10.0);
  test_tearDown("test", 1);

  printf("Test Beta Sampler: . . .\n");
  model_dim = 1;
  test_setUp(1, &model_dim, logp_beta_sampler, init_beta_sampler);
  pass |= test_sampler(0.5, 0.5, 0.0, 1.0);
  test_tearDown("test", 1);

  printf("Test Normal Param Estimation: . . .\n");
  model_dim = 2;
  test_setUp(1, &model_dim, logp_normal_params, init_normal_params);
  pass |= test_dist_params(0.2, 0.5);
  test_tearDown("test", 1);

  printf("Test Beta Param Estimation: . . .\n");
  model_dim = 2;
  test_setUp(1, &model_dim, logp_beta_params, init_normal_params);
  pass |= test_dist_params(4.5, 5.0);
  test_tearDown("test", 1);

  printf("Test Gamma Param Estimation: . . .\n");
  model_dim = 2;
  test_setUp(1, &model_dim, logp_gamma_params, init_normal_params);
  pass |= test_dist_params(7.0, 14.5);
  test_tearDown("test", 1);

  printf("Test Gamma-Beta Model Selection: . . .\n");
  model_dims[0] = 2;
  model_dims[1] = 2;
  test_setUp(2, model_dims, logp_gamma_beta, init_gamma_beta);
  pass |= test_two_models(7.0, 14.5, 4.7, 4.8, 0.37);
  test_tearDown("test", 2);

  printf("Test Normal-Beta Model Selection: . . .\n");
  model_dims[0] = 2;
  model_dims[1] = 2;
  test_setUp(2, model_dims, logp_normal_beta, init_normal_beta);
  pass |= test_two_models(0.2, 0.5, 4.7, 4.8, 0.95);
  test_tearDown("test", 2);

  printf("Test Normal-Gamma Model Selection: . . .\n");
  model_dims[0] = 2;
  model_dims[1] = 2;
  test_setUp(2, model_dims, logp_normal_gamma, init_normal_gamma);
  pass |= test_two_models(0.2, 0.5, 7.1, 14.5, 0.97);
  test_tearDown("test", 2);

  return pass;
}

int test_setUp(int nmodels, int *model_dims, targetFunc logposterior,
               rwmInitFunc initRWM) {
  amSampler am;
  initAMSampler(&am, nmodels, model_dims, logposterior, initRWM);
  int ncond_prob_sweeps = 100000; // 1E5
  estimate_conditional_probs(&am, ncond_prob_sweeps);
  report_cond_prob_estimation("test", am);
  int nburn_sweeps = 10000; // 1E4
  burn_samples(&am, nburn_sweeps);
  int nsweeps = 100000; // 1E5
  rjmcmc_samples(&am, nsweeps);
  report_rjmcmc_run("test", am, 0, ncond_prob_sweeps, nsweeps);
  freeAMSampler(&am);
  return EXIT_SUCCESS;
}

int test_tearDown(char *filename, int nmodels) {
  char fname_buffer[50];
  sprintf(fname_buffer, "%s_ac.data", filename);
  remove(fname_buffer);
  sprintf(fname_buffer, "%s_cf.data", filename);
  remove(fname_buffer);
  sprintf(fname_buffer, "%s_log.data", filename);
  remove(fname_buffer);
  sprintf(fname_buffer, "%s_mix.data", filename);
  remove(fname_buffer);
  sprintf(fname_buffer, "%s_adapt.data", filename);
  remove(fname_buffer);
  sprintf(fname_buffer, "%s_k.data", filename);
  remove(fname_buffer);
  sprintf(fname_buffer, "%s_lp.data", filename);
  remove(fname_buffer);
  sprintf(fname_buffer, "%s_pk.data", filename);
  remove(fname_buffer);
  for (int i = 0; i < nmodels; ++i) {
    sprintf(fname_buffer, "%s_theta%d.data", filename, i + 1);
    remove(fname_buffer);
  }
  return EXIT_SUCCESS;
}

/************************************************
 *                                               *
 *                 Test Checks                   *
 *                                               *
 ************************************************/

int test_sampler(double true_mean, double true_sigma, double lower,
                 double upper) {
  printf("Test Distribution Sampler:......");
  FILE *fp = fopen("test_theta1.data", "r");
  if (fp == NULL) {
    return 1;
  }
  int ndraws = 100000;
  double mean = 0.0;
  double sumsq = 0.0;
  for (int i = 0; i < ndraws; i++) {
    double datum;
    fscanf(fp, "%lf", &datum);
    mean += datum;
    sumsq += datum * datum;
    if (datum > upper || datum < lower) {
      int pass = 0; // didn't pass the test.
      printf("FAIL\nValue outside range encountered: %lf\n", datum);
      return !pass;
    }
  }
  fclose(fp);
  mean /= ndraws;
  double sigma = sqrt((sumsq - mean * mean) / (ndraws - 1));
  double tol = 0.5;
  int pass = fabs(mean - true_mean) < tol && fabs(sigma - true_sigma) < tol;
  if (!pass) {
    printf("FAIL\nmean=%lf, sigma = %lf\n", mean, sigma);
  } else {
    printf("OK\n");
  }
  return !pass;
}

int test_dist_params(double true_param1, double true_param2) {
  printf("Test Distribution Parameters:......");
  FILE *fp = fopen("test_theta1.data", "r");
  if (fp == NULL) {
    return 1;
  }
  int ndraws = 100000;
  double p1_mean = 0.0;
  double p2_mean = 0.0;
  for (int i = 0; i < ndraws; i++) {
    double p1, p2;
    fscanf(fp, "%lf %lf", &p1, &p2);
    p1_mean += p1;
    p2_mean += p2;
  }
  fclose(fp);
  p1_mean /= ndraws;
  p2_mean /= ndraws;

  double tol = 0.2;
  int pass =
      fabs(p1_mean - true_param1) < tol && fabs(p2_mean - true_param2) < tol;
  if (!pass) {
    printf("FAIL\nparam1=%lf, param2 = %lf\n", p1_mean, p2_mean);
  } else {
    printf("OK\n");
  }
  return !pass;
}

int test_two_models(double true_k1_p1, double true_k1_p2, double true_k2_p1,
                    double true_k2_p2, double true_k1_frac) {
  printf("Test Model Selection:......");
  char fname[50];
  int k_count[2];
  double p1_mean[2], p2_mean[2];
  for (int i = 0; i < 2; ++i) {
    sprintf(fname, "test_theta%d.data", i + 1);
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
      return 1;
    }
    double p1, p2;
    k_count[i] = 0;
    p1_mean[i] = 0.0;
    p2_mean[i] = 0.0;
    while (fscanf(fp, "%lf %lf", &p1, &p2) != EOF) {
      p1_mean[i] += p1;
      p2_mean[i] += p2;
      (k_count[i])++;
    }
    fclose(fp);
    p1_mean[i] /= k_count[i];
    p2_mean[i] /= k_count[i];
  }
  double tol = 0.2;
  int pass1 = fabs(p1_mean[0] - true_k1_p1) < tol &&
              fabs(p2_mean[0] - true_k1_p2) < tol;
  if (!pass1) {
    printf("FAIL\nModel 1: (p1=%lf, p2=%lf)\n", p1_mean[0], p2_mean[0]);
  }
  int pass2 = fabs(p1_mean[1] - true_k2_p1) < tol &&
              fabs(p2_mean[1] - true_k2_p2) < tol;
  if (!pass2) {
    printf("FAIL\nModel 2: (p1=%lf, p2=%lf)\n", p1_mean[1], p2_mean[1]);
  }
  double k1_frac = (double)k_count[0] / (k_count[0] + k_count[1]);
  int pass3 = fabs(k1_frac - true_k1_frac) < tol;
  if (!pass3) {
    printf("FAIL\nModel 1 probability: %lf\n", k1_frac);
  }
  if (pass1 && pass2 && pass3) {
    printf("OK\n");
  }
  return !(pass1 && pass2 && pass3);
}

/************************************************
 *                                               *
 *         Distributions to sample from          *
 *                                               *
 ************************************************/

double logp_truncnormal_sampler(int model_k, int mdim, double *xp) {
  double x = *xp;
  double a = 0.0;
  double b = 10.0;
  if (x <= a || x >= b) {
    return -DBL_MAX;
  }
  double prob;
  double x0 = 1.0;
  double sigma = 1.0;
  prob = -(x - x0) * (x - x0) / (2.0 * sigma * sigma);
  return prob;
}

double logp_normal_sampler(int model_k, int mdim, double *xp) {
  double x = *xp;
  double prob;
  double x0 = 0.5;
  double sigma = 1.0;
  prob = -(x - x0) * (x - x0) / (2.0 * sigma * sigma);
  return prob;
}

double logp_beta_sampler(int model_k, int mdim, double *xp) {
  double x = *xp;
  if (x <= 0.0 || x >= 1.0) {
    return -DBL_MAX;
  }
  double alpha = 2.0;
  double beta = 2.0;
  double prod = (alpha - 1.0) * log(x) + (beta - 1.0) * log(1.0 - x);
  prod += loggamma(alpha + beta) - loggamma(alpha) - loggamma(beta);
  return prod;
}

/************************************************
 *                                               *
 *  Distributions with indeterminate parameters  *
 *                                               *
 ************************************************/

double logp_normal_params(int model_k, int mdim, double *params) {
  double sigma = params[0];
  double x0 = params[1];
  double prod = 0;
  for (int i = 0; i < nsamples; i++) {
    double x = data_samples[i];
    prod += -(x - x0) * (x - x0);
  }
  prod = -nsamples * log(sigma) + prod / (2.0 * sigma * sigma);
  return prod;
}

double logp_beta_params(int model_k, int mdim, double *params) {
  double alpha = params[0];
  double beta = params[1];
  if (alpha <= 0.0 || beta <= 0.0) {
    return -DBL_MAX;
  }
  double prod = 0.0;
  for (int i = 0; i < nsamples; i++) {
    double x = data_samples[i];
    prod += (alpha - 1.0) * log(x) + (beta - 1.0) * log(1.0 - x);
  }
  prod +=
      nsamples * (loggamma(alpha + beta) - loggamma(alpha) - loggamma(beta));
  return prod;
}

double logp_gamma_params(int model_k, int mdim, double *params) {
  double alpha = params[0];
  double beta = params[1];
  double prod = 0.0;
  for (int i = 0; i < nsamples; ++i) {
    double x = data_samples[i];
    prod += (alpha - 1.0) * log(x) - beta * x;
  }
  prod += nsamples * (alpha * log(beta) - loggamma(alpha));
  return prod;
}

/************************************************
 *                                               *
 *              Two-model RJMCMC                 *
 *                                               *
 ************************************************/

double logp_gamma_beta(int model_k, int mdim, double *params) {
  if (model_k == 0) {
    return logp_gamma_params(0, 2, params);
  } else if (model_k == 1) {
    return logp_beta_params(0, 2, params);
  }
  return 0.0;
}

double logp_normal_beta(int model_k, int mdim, double *params) {
  if (model_k == 0) {
    return logp_normal_params(0, 2, params);
  } else if (model_k == 1) {
    return logp_beta_params(0, 2, params);
  }
  return 0.0;
}

double logp_normal_gamma(int model_k, int mdim, double *params) {
  if (model_k == 0) {
    return logp_normal_params(0, 2, params);
  } else if (model_k == 1) {
    return logp_gamma_params(0, 2, params);
  }
  return 0.0;
}

/************************************************
 *                                               *
 *           Initial values for RWM              *
 *                                               *
 ************************************************/

void init_normal_sampler(int model_k, int mdim, double *xp) { *xp = 0.5; }
void init_truncnormal_sampler(int model_k, int mdim, double *xp) { *xp = 1.0; }
void init_beta_sampler(int model_k, int mdim, double *xp) { *xp = 0.5; }
void init_normal_params(int model_k, int mdim, double *xp) {
  xp[0] = 0.5;
  xp[1] = 0.5;
}
void init_beta_params(int model_k, int mdim, double *xp) {
  xp[0] = 2.0;
  xp[1] = 2.0;
}
void init_gamma_params(int model_k, int mdim, double *xp) {
  xp[0] = 9.0;
  xp[1] = 2.0;
}
void init_gamma_beta(int model_k, int mdim, double *xp) {
  if (model_k == 0) {
    init_gamma_params(0, 2, xp);
    return;
  } else if (model_k == 1) {
    init_beta_params(1, 2, xp);
  }
}
void init_normal_beta(int model_k, int mdim, double *xp) {
  if (model_k == 0) {
    init_normal_params(0, 2, xp);
    return;
  } else if (model_k == 1) {
    init_beta_params(1, 2, xp);
  }
}
void init_normal_gamma(int model_k, int mdim, double *xp) {
  if (model_k == 0) {
    init_normal_params(0, 2, xp);
    return;
  } else if (model_k == 1) {
    init_gamma_params(1, 2, xp);
  }
}