/*
 * Copyright 2015 Ettus Research LLC
 * Copyright 2018 Ettus Research, a National Instruments Company
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/*
Comentar Aqui....
*/


#include <uhd.h>

#include "getopt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fftw3.h>


#define EXECUTE_OR_GOTO(label, ...) \
    if(__VA_ARGS__){ \
        return_code = EXIT_FAILURE; \
        goto label; \
    }
/*
Comentar Aqui....
*/
void print_help(void)
{
    fprintf(stderr, "rx_samples_c - A simple RX example using UHD's C API\n\n"

            "Options:\n"
            "    -a (device args)\n"
            "    -l (frequency local oscilator in Hz)\n"
            "    -f (frequency RF in Hz)\n"
            "    -r (sample rate in Hz)\n"
            "    -i (integration time in seconds)\n"
            "    -t (observation time in minutes)\n"
            "    -g (gain)\n"
            "    -n (number of FFT points)\n"
            "    -o (label, default = \"out\")\n"
            "    -v (enable verbose prints)\n"
            "    -h (print this help message)\n");

};
/*
Comentar Aqui....
*/
int N=4096;
float *v_fft;
float *v_fft_suma;
float pot_t_int=0;
float pot_t_obs=0;
int i=0,ii=0;
int tiempo;
int q=0;
//tiempo de integracion
int tiempo_integracion=60; //en segundos
// tiempo de observacion
int tiempo_minutos=1; // tiempon de observacion en minutos
int num_rx_samps;
FILE *p1;
FILE *p2;
FILE *p3;
FILE *p4;

int main(int argc, char* argv[])
{
    fftw_complex in[N], out[N],  buff_aux0[N];
    fftw_plan p;

    if(uhd_set_thread_priority(uhd_default_thread_priority, true))
    {
        fprintf(stderr, "Unable to set thread priority. Continuing anyway.\n");
    }

    int option = 0;
    double freq = 150e6;
    double rate = 20e6;
    double gain = 20.0;
    double freq_local=1570.407e6;
    double freq_RF=1422e6;
    char* device_args = NULL;
    size_t channel = 0;
    char* filename = "out";
    char filename1[150];
    char filename2[150];
    char filename3[150];
    char filename4[150];
    size_t n_samples = N;
    bool verbose = false;
    int return_code = EXIT_SUCCESS;
    bool custom_filename = false;
    char error_string[512];

    // Process options
    while((option = getopt(argc, argv, "a:l:f:r:i:t:g:n:o:vh")) != -1)
    {
        switch(option)
        {
        case 'a':
            device_args = strdup(optarg);
            break;

        case 'l':
            freq_local = atof(optarg);
            break;

        case 'f':
            freq_RF = atof(optarg);
            if (freq_RF >1490e6 || freq_RF <1350e6)
            {
                printf("ERROR:frecuencia RF invalida(debe estar entre 1490Mhz y 1350Mhz)");
                return 1;
            }
            break;

        case 'r':
            rate = atof(optarg);
            if(rate >56e6 || freq_RF <1e6)
            {
                printf("ERROR:ancho de banda invalido(debe estar entre 1Mhz y 56Mhz)");
                return 1;
            }

            break;

        case 'i':
            tiempo_integracion = atoi(optarg);
            break;

        case 't':
            tiempo_minutos = atoi(optarg);
            break;

        case 'g':
            gain = atof(optarg);
            break;

        case 'n':
            N = atoi(optarg);
            break;

        case 'o':
            filename = strdup(optarg);
            custom_filename = true;
            break;

        case 'v':
            verbose = true;
            break;

        case 'h':
            print_help();
            goto free_option_strings;

        default:
            print_help();
            return_code = EXIT_FAILURE;
            goto free_option_strings;
        }
    }
/*
Comentar Aqui....
*/
    freq=freq_local-freq_RF;
/*
Comentar Aqui....
*/
    strcpy(filename1,filename);
    strcat(filename1,"_fft_tint.dat");

    strcpy(filename2,filename);
    strcat(filename2,"_fft_tobs.dat");

    strcpy(filename3,filename);
    strcat(filename3,"_pot_tint.dat");

    strcpy(filename4,filename);
    strcat(filename4,"_pot_tobs.dat");

    p1 = fopen(filename1,"w");
    p2 = fopen(filename2,"w");
    p3 = fopen(filename3,"w");
    p4 = fopen(filename4,"w");

    if (p1==NULL && p2==NULL && p3==NULL && p4==NULL)
    {
        printf("ERROR(No se puede abrir el archivo)");
        exit(1);
    }
/*
Comentar Aqui....
*/
    // header
    fprintf(p1, "#FREQ_RF,%f\n", freq_RF);
    fprintf(p1, "#FREQ_OL,%f\n", freq_local);
    fprintf(p1, "#GAIN,%f\n",gain);
    fprintf(p1, "#FFT_CHANNELS,%d\n",N);
    fprintf(p1, "#T_INT[seg],%d\n",tiempo_integracion);
    fprintf(p1, "#T_OBS[min],%d\n",tiempo_minutos);
    fprintf(p1, "#RATE,%f\n",rate);
    fflush(p1);

    fprintf(p2, "#FREQ_RF,%f\n", freq_RF);
    fprintf(p2, "#FREQ_OL,%f\n", freq_local);
    fprintf(p2, "#GAIN,%f\n",gain);
    fprintf(p2, "#FFT_CHANNELS,%d\n",N);
    fprintf(p2, "#T_INT[seg],%d\n",tiempo_integracion);
    fprintf(p2, "#T_OBS[min],%d\n",tiempo_minutos);
    fprintf(p2, "#RATE,%f\n",rate);
    fflush(p2);

    fprintf(p3, "#FREQ_RF,%f\n", freq_RF);
    fprintf(p3, "#FREQ_OL,%f\n", freq_local);
    fprintf(p3, "#GAIN,%f\n",gain);
    fprintf(p3, "#FFT_CHANNELS,%d\n",N);
    fprintf(p3, "#T_INT[seg],%d\n",tiempo_integracion);
    fprintf(p3, "#T_OBS[min],%d\n",tiempo_minutos);
    fprintf(p3, "#RATE,%f\n",rate);
    fflush(p3);

    fprintf(p4, "#FREQ_RF,%f\n", freq_RF);
    fprintf(p4, "#FREQ_OL,%f\n", freq_local);
    fprintf(p4, "#GAIN,%f\n",gain);
    fprintf(p4, "#FFT_CHANNELS,%d\n",N);
    fprintf(p4, "#T_INT[seg],%d\n",tiempo_integracion);
    fprintf(p4, "#T_OBS[min],%d\n",tiempo_minutos);
    fprintf(p4, "#RATE,%f\n",rate);
    fflush(p4);

 /*
Comentar Aqui....
*/
    v_fft=malloc( sizeof(float)*N);// hago memoria para sumar los valores de las fft
    v_fft_suma=malloc( sizeof(float)*N);
    for (i = 0; i <N; i++) //inicializo en 0 a los vectores creados arriba
    {
        v_fft[i] = 0;
    }
    for (i = 0; i <N; i++)
    {
        v_fft_suma[i]=0;
    }

 /*
Comentar Aqui....
*/

    if (!device_args)
        device_args = strdup("");

    // Create USRP
    uhd_usrp_handle usrp;
    fprintf(stderr, "Creating USRP with args \"%s\"...\n", device_args);
    EXECUTE_OR_GOTO(free_option_strings,
                    uhd_usrp_make(&usrp, device_args)
                   )

    // Create RX streamer
    uhd_rx_streamer_handle rx_streamer;
    EXECUTE_OR_GOTO(free_usrp,
                    uhd_rx_streamer_make(&rx_streamer)
                   )

    // Create RX metadata
    uhd_rx_metadata_handle md;
    EXECUTE_OR_GOTO(free_rx_streamer,
                    uhd_rx_metadata_make(&md)
                   )

    // Create other necessary structs
    uhd_tune_request_t tune_request =
    {
        .target_freq = freq,
        .rf_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
        .dsp_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
    };
    uhd_tune_result_t tune_result;

    uhd_stream_args_t stream_args =
    {
        .cpu_format = "sc16",
        .otw_format = "sc8",
        .args = "",
        .channel_list = &channel,
        .n_channels = 0
    };


    uhd_stream_cmd_t stream_cmd =
    {
        .stream_mode = UHD_STREAM_MODE_START_CONTINUOUS,
        .num_samps = N,
        .stream_now = true
    };

    size_t samps_per_buff;
    short *buff = NULL;
    void **buffs_ptr = NULL;
    //FILE *fp = NULL;
    size_t num_acc_samps = 0;

    // Set rate
    fprintf(stderr, "Setting RX Rate: %f...\n", rate);
    EXECUTE_OR_GOTO(free_rx_metadata,
                    uhd_usrp_set_rx_rate(usrp, rate, channel)
                   )

    // See what rate actually is
    EXECUTE_OR_GOTO(free_rx_metadata,
                    uhd_usrp_get_rx_rate(usrp, channel, &rate)
                   )
    fprintf(stderr, "Actual RX Rate: %f...\n", rate);

    // Set gain
    fprintf(stderr, "Setting RX Gain: %f dB...\n", gain);
    EXECUTE_OR_GOTO(free_rx_metadata,
                    uhd_usrp_set_rx_gain(usrp, gain, channel, "")
                   )

    // See what gain actually is
    EXECUTE_OR_GOTO(free_rx_metadata,
                    uhd_usrp_get_rx_gain(usrp, channel, "", &gain)
                   )
    fprintf(stderr, "Actual RX Gain: %f...\n", gain);

    // Set frequency
    fprintf(stderr, "Setting RX frequency: %f MHz...\n", freq/1e6);
    EXECUTE_OR_GOTO(free_rx_metadata,
                    uhd_usrp_set_rx_freq(usrp, &tune_request, channel, &tune_result)
                   )

    // See what frequency actually is
    EXECUTE_OR_GOTO(free_rx_metadata,
                    uhd_usrp_get_rx_freq(usrp, channel, &freq)
                   )
    fprintf(stderr, "Actual RX frequency: %f MHz...\n", freq / 1e6);

    // Set up streamer
    stream_args.channel_list = &channel;
    EXECUTE_OR_GOTO(free_rx_streamer,
                    uhd_usrp_get_rx_stream(usrp, &stream_args, rx_streamer)
                   )

    // Set up buffer
    samps_per_buff=N; // seteo el buffer para enviar paquetes de 1024 muestras.
    EXECUTE_OR_GOTO(free_rx_streamer,
                    uhd_rx_streamer_max_num_samps(rx_streamer, &samps_per_buff)
                   )
    fprintf(stderr, "Buffer size in samples: %zu\n", samps_per_buff);
    buff = malloc(samps_per_buff * 2 * sizeof(short));
    buffs_ptr = (void**)&buff;

    // Issue stream command
    fprintf(stderr, "Issuing stream command.\n");
    EXECUTE_OR_GOTO(free_buffer,
                    uhd_rx_streamer_issue_stream_cmd(rx_streamer, &stream_cmd)
                   )

/*
Comentar Aqui....
*/
    // Set up file output
    //fp = fopen(filename, "w");
 /*
Comentar Aqui....
*/
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_MEASURE);

// iniciar variables en 0
// Tomar de a 1024 muestars (samps_per_buff) - calcular la FFT y sumarlas..
// guardamos el promedio total.
// n_samples es la cantidad total de repeticiones de 1024 muestras por paquete que se debe repetir 97656 veces para llegar a los 10 segundos aproxiamdamente.
// 10MHz rate * 10 seg / 1024 = 97656
/*
Comentar Aqui....
*/
    tiempo = 60*tiempo_minutos/tiempo_integracion;
    n_samples= tiempo_integracion*rate/N;
    samps_per_buff=N;
    printf("start reading\n");
//-----------------------------------------------

    // Actual streaming
/*
Comentar Aqui....
*/
    while(q<tiempo)
    {
        while (num_acc_samps < n_samples)
        {
            size_t num_rx_samps = 0;
            // Realizo la adquisicion de 1024 muestras
            EXECUTE_OR_GOTO(close_file,
                            uhd_rx_streamer_recv(rx_streamer, buffs_ptr, samps_per_buff, &md, 3.0, false, &num_rx_samps)
                           )
            if(num_rx_samps==samps_per_buff)  // reviso que la cantidad de puntos de la fft ingresada coincida con la cantidad de puntos q devuelve la funcion
            {
                ii=0;

                for(i=0; i<(int)N*2; i+=2)
                {
                    in[ii][0]=(float)buff[i]/255.0;
                    in[ii][1]=(float)buff[i+1]/255.0;
                    ii++;
                }
                // Calcular la FFT de 1024 puntos

                fftw_execute(p);

                for (i = N/2; i < N; i++)
                {
                    buff_aux0[i-(N/2)][0]=out[i][0];
                    buff_aux0[i-(N/2)][1]=out[i][1];
                }
                for (i = 0; i < N/2; i++)
                {
                    buff_aux0[i+N/2][0]=out[i][0];
                    buff_aux0[i+N/2][1]=out[i][1];
                }
                for (i = 0; i <N; i++)
                {
                    v_fft[i] = ((buff_aux0[i][0]*buff_aux0[i][0]) + (buff_aux0[i][1]*buff_aux0[i][1]))+v_fft[i];

                }

                uhd_rx_metadata_error_code_t error_code;
                EXECUTE_OR_GOTO(close_file,
                                uhd_rx_metadata_error_code(md, &error_code)
                               )
                if(error_code != UHD_RX_METADATA_ERROR_CODE_NONE)
                {
                    fprintf(stderr, "Error code 0x%x was returned during streaming. Aborting.\n", return_code);
                    //goto close_file;
                }

                // Handle data
                //fwrite(buff, sizeof(float) * 2, num_rx_samps, fp);
                if (verbose)
                {
                    int64_t full_secs;
                    double frac_secs;
                    uhd_rx_metadata_time_spec(md, &full_secs, &frac_secs);
                    fprintf(stderr, "Received packet: %zu samples, %.f full secs, %f frac secs\n",
                            num_rx_samps,
                            difftime(full_secs, (int64_t) 0),
                            frac_secs);
                }

                num_acc_samps += 1;
            }
        } //si sale de este while es porq se cumplio un t de integracion
 /*
Comentar Aqui....
*/
        num_acc_samps=0;

        for (i = 0; i <N; i++)
        {
            v_fft[i] = v_fft[i]/n_samples;
            fprintf(p1,"%f,",v_fft[i]);

        }
        fprintf(p1,"\n");
        fflush(p1);

        for (i = 0; i <N; i++)
        {
            pot_t_int+=v_fft[i];
        }
        fprintf(p3,"%f,",pot_t_int);
        fprintf(p3,"\n");
        fflush(p3);

        for(i=0; i<N; i++)
        {
            v_fft_suma[i]+= v_fft[i];//sumo todas las lineas espctrales de los t_int punto a punto,utilizo a v_fft_suma como variable aux porque sobre v_fft se sobre escriben datos
        }
        q++;
    } //si salgo de aca es porq recorri todos los tiempos de integracion
//**********************************************************************************************

/*
Comentar Aqui....
*/
    for(i=0; i<N; i++)
    {
        v_fft_suma[i]=v_fft_suma[i]/tiempo;
        fprintf(p2,"%f,",v_fft_suma[i]);
    }
    fflush(p2);
    for (i = 0; i <N; i++)
    {
        pot_t_obs+=v_fft_suma[i];
    }
    fprintf(p4,"%f,",pot_t_obs);
    fflush(p4);

 /*
Comentar Aqui....
*/
    // Cleanup
close_file:

    fclose(p1);
    fclose(p2);
    fclose(p3);
    fclose(p4);

free_buffer:
    if(buff)
    {
        if(verbose)
        {
            fprintf(stderr, "Freeing buffer.\n");
        }
        //free(buff);
    }
    fprintf(stderr, "Freeing buffer 2.\n");
    buff = NULL;
    buffs_ptr = NULL;

free_rx_streamer:
    if(verbose)
    {
        fprintf(stderr, "Cleaning up RX streamer.\n");
    }
    uhd_rx_streamer_free(&rx_streamer);

free_rx_metadata:
    if(verbose)
    {
        fprintf(stderr, "Cleaning up RX metadata.\n");
    }
    uhd_rx_metadata_free(&md);

free_usrp:
    if(verbose)
    {
        fprintf(stderr, "Cleaning up USRP.\n");
    }
    if(return_code != EXIT_SUCCESS && usrp != NULL)
    {
        uhd_usrp_last_error(usrp, error_string, 512);
        fprintf(stderr, "USRP reported the following error: %s\n", error_string);
    }
    uhd_usrp_free(&usrp);

free_option_strings:
    if(device_args)
    {
        free(device_args);
    }
    if(custom_filename)
    {
        free(filename);
    }

    fprintf(stderr, (return_code ? "Failure\n" : "Success\n"));



    return 1;
}
