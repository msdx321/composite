#include <string.h>
#include "osapi.h"
#include "ostask.h"
#include <sl.h>

#define CFE_PSP_SENSORQ_NAME "SENSORTRACE_QUEUE"
#define CFE_PSP_SENSORQ_DEPTH 20
#define CFE_PSP_SENSOR_DATASZ 550

static uint32 SensorQid = 0;

static const char *SensorTraceDump[] = {
"    5.00000000e-01    -1.16562879e-05    -6.21879309e+06     2.69626818e+06     7.66858788e+03    -1.20994435e-08     5.24552142e-09     4.42000922e-32    -1.13136970e-03    -4.20869496e-21     8.35998956e-01    -4.71843178e-13     7.18814907e-13    -5.48731033e-01    -9.99633870e-01     2.48381680e-02     1.07327611e-02    -9.99633870e-01    -1.97274007e-02     1.85190824e-02     6.86409657e-07    -2.17688308e-05     2.99212043e-05     0.00000000e+00     0.00000000e+00     0.00000000e+00 1 ",
"    5.00000000e-01     3.83429372e+03    -6.21879210e+06     2.69626775e+06     7.66858665e+03     3.98001927e+00    -1.72560803e+00    -1.12467509e-05    -1.04919873e-03     3.03883477e-06     8.35999696e-01     1.49250610e-04    -2.28077568e-04    -5.48729837e-01    -9.99633872e-01     2.48380768e-02     1.07327215e-02    -9.99623643e-01    -1.97266215e-02     1.90638987e-02     6.73380807e-07    -2.17673927e-05     2.99223854e-05    -3.32270233e-09    -1.67697274e-01    -1.12728552e-18 1 ",
"    5.00000000e-01     7.66858623e+03    -6.21878911e+06     2.69626646e+06     7.66858297e+03     7.96003727e+00    -3.45121552e+00    -2.08418686e-05    -9.73228251e-04     5.63635662e-06     8.36001813e-01     2.87065688e-04    -4.40015563e-04    -5.48726428e-01    -9.99633875e-01     2.48379856e-02     1.07326820e-02    -9.99613922e-01    -1.97245359e-02     1.95690865e-02     6.59135175e-07    -2.17661089e-05     2.99234598e-05    -2.21154547e-03    -3.22997260e-01     8.30418050e-04 1 ",
"    5.00000000e-01     1.15028763e+04    -6.21878414e+06     2.69626430e+06     7.66857683e+03     1.19400527e+01    -5.17682190e+00    -2.89100792e-05    -9.03197775e-04     7.82629208e-06     8.36005099e-01     4.14367105e-04    -6.37024142e-04    -5.48721146e-01    -9.99633878e-01     2.48378944e-02     1.07326425e-02    -9.99604702e-01    -1.97213452e-02     2.00376965e-02     6.43768117e-07    -2.17649553e-05     2.99244411e-05    -6.38743221e-03    -4.66400444e-01     2.39985524e-03 1 ",
"    5.00000000e-01     1.53371627e+04    -6.21877717e+06     2.69626128e+06     7.66856825e+03     1.59200644e+01    -6.90242662e+00    -3.69810339e-05    -8.33167283e-04     1.00099182e-05     8.36009462e-01     5.31605948e-04    -8.19695374e-04    -5.48714156e-01    -9.99633880e-01     2.48378031e-02     1.07326030e-02    -9.99596000e-01    -1.97171490e-02     2.04712214e-02     6.27326120e-07    -2.17639206e-05     2.99253351e-05    -1.05633189e-02    -6.09803629e-01     3.96929244e-03 1 ",
"    5.00000000e-01     1.91714441e+04    -6.21876822e+06     2.69625740e+06     7.66855720e+03     1.99000709e+01    -8.62802914e+00    -4.36445771e-05    -7.68820325e-04     1.18181868e-05     8.36014812e-01     6.39212268e-04    -9.88596791e-04    -5.48705611e-01    -9.99633883e-01     2.48377119e-02     1.07325634e-02    -9.99587832e-01    -1.97120382e-02     2.08710895e-02     6.09853739e-07    -2.17629940e-05     2.99261468e-05    -1.64694779e-02    -7.41804022e-01     6.19138659e-03 1 ",
"    5.00000000e-01     2.30057194e+04    -6.21875727e+06     2.69625265e+06     7.66854371e+03     2.38800711e+01    -1.03536289e+01    -4.88545066e-05    -7.10342672e-04     1.32398847e-05     8.36020962e-01     7.38058176e-04    -1.14488336e-03    -5.48695814e-01    -9.99633886e-01     2.48376206e-02     1.07325239e-02    -9.99580172e-01    -1.97061906e-02     2.12402027e-02     5.91441390e-07    -2.17621539e-05     2.99268883e-05    -2.41621000e-02    -8.62030900e-01     9.08556687e-03 1 ",
"    5.00000000e-01     2.68399874e+04    -6.21874434e+06     2.69624704e+06     7.66852776e+03     2.78600636e+01    -1.20792253e+01    -5.40665660e-05    -6.51864994e-04     1.46567860e-05     8.36027816e-01     8.28587300e-04    -1.28914148e-03    -5.48684921e-01    -9.99633888e-01     2.48375293e-02     1.07324844e-02    -9.99573021e-01    -1.96996976e-02     2.15800354e-02     5.72135114e-07    -2.17613895e-05     2.99275650e-05    -3.18547221e-02    -9.82257777e-01     1.19797471e-02 1 ",
"    5.00000000e-01     3.06742468e+04    -6.21872941e+06     2.69624057e+06     7.66850935e+03     3.18400472e+01    -1.38048179e+01    -5.79392678e-05    -5.99004527e-04     1.57178866e-05     8.36035289e-01     9.11222713e-04    -1.42193336e-03    -5.48673075e-01    -9.99633891e-01     2.48374380e-02     1.07324449e-02    -9.99566376e-01    -1.96926433e-02     2.18919987e-02     5.51978805e-07    -2.17606906e-05     2.99281818e-05    -4.11126849e-02    -1.09119790e+00     1.54645758e-02 1 ",
"    5.00000000e-01     3.45084964e+04    -6.21871250e+06     2.69623324e+06     7.66848849e+03     3.58200207e+01    -1.55304061e+01    -6.05977696e-05    -5.51479414e-04     1.64567317e-05     8.36043209e-01     9.86786306e-04    -1.54435718e-03    -5.48660545e-01    -9.99633894e-01     2.48373467e-02     1.07324054e-02    -9.99560200e-01    -1.96851835e-02     2.21788426e-02     5.31058021e-07    -2.17600379e-05     2.99287493e-05    -5.16961127e-02    -1.18939614e+00     1.94519858e-02 1 ",
"    5.00000000e-01     3.83427349e+04    -6.21869359e+06     2.69622504e+06     7.66846518e+03     3.97999826e+01    -1.72559893e+01    -6.32578353e-05    -5.03954268e-04     1.71921665e-05     8.36051497e-01     1.05567805e-03    -1.65694815e-03    -5.48647459e-01    -9.99633897e-01     2.48372554e-02     1.07323660e-02    -9.99554481e-01    -1.96773932e-02     2.24419067e-02     5.09414497e-07    -2.17594224e-05     2.99292719e-05    -6.22795404e-02    -1.28759438e+00     2.34393959e-02 1 ",
"    5.00000000e-01     4.21769611e+04    -6.21867270e+06     2.69621598e+06     7.66843941e+03     4.37799319e+01    -1.89815669e+01    -6.46690774e-05    -4.61918824e-04     1.75970418e-05     8.36060068e-01     1.11830930e-03    -1.76025710e-03    -5.48633952e-01    -9.99633899e-01     2.48371641e-02     1.07323265e-02    -9.99549207e-01    -1.96693488e-02     2.26825695e-02     4.87091033e-07    -2.17588346e-05     2.99297543e-05    -7.42247274e-02    -1.37474118e+00     2.79419663e-02 1 ",
"    5.00000000e-01     4.60111739e+04    -6.21864981e+06     2.69620606e+06     7.66841119e+03     4.77598671e+01    -2.07071385e+01    -6.49516368e-05    -4.25095422e-04     1.77034994e-05     8.36068762e-01     1.17547939e-03    -1.85535951e-03    -5.48620270e-01    -9.99633902e-01     2.48370727e-02     1.07322870e-02    -9.99544329e-01    -1.96611936e-02     2.29035173e-02     4.64171190e-07    -2.17582566e-05     2.99302065e-05    -8.73042002e-02    -1.45137444e+00     3.28763371e-02 1 ",
"    5.00000000e-01     4.98453719e+04    -6.21862494e+06     2.69619528e+06     7.66838052e+03     5.17397870e+01    -2.24327035e+01    -6.52352184e-05    -3.88271981e-04     1.78078664e-05     8.36077502e-01     1.22757705e-03    -1.94277955e-03    -5.48606534e-01    -9.99633905e-01     2.48369813e-02     1.07322476e-02    -9.99539829e-01    -1.96529960e-02     2.31060581e-02     4.40695532e-07    -2.17576800e-05     2.99306327e-05    -1.00383673e-01    -1.52800770e+00     3.78107078e-02 1 ",
"    5.00000000e-01     5.36795540e+04    -6.21859807e+06     2.69618363e+06     7.66834739e+03     5.57196904e+01    -2.41582613e+01    -6.45037528e-05    -3.56390180e-04     1.76441487e-05     8.36086220e-01     1.27496928e-03    -2.02301521e-03    -5.48592849e-01    -9.99633907e-01     2.48368900e-02     1.07322081e-02    -9.99535687e-01    -1.96448169e-02     2.32914322e-02     4.16702586e-07    -2.17570971e-05     2.99310368e-05    -1.14385341e-01    -1.59465255e+00     4.30992899e-02 1 ",
"    5.00000000e-01     5.75137189e+04    -6.21856922e+06     2.69617112e+06     7.66831181e+03     5.96995760e+01    -2.58838113e+01    -6.27331391e-05    -3.29574665e-04     1.72066803e-05     8.36094774e-01     1.31839858e-03    -2.09707551e-03    -5.48579432e-01    -9.99633910e-01     2.48367986e-02     1.07321686e-02    -9.99531853e-01    -1.96367793e-02     2.34621506e-02     3.92270167e-07    -2.17564926e-05     2.99314276e-05    -1.29327834e-01    -1.65105739e+00     4.87485442e-02 1 ",
"    5.00000000e-01     6.13478654e+04    -6.21853837e+06     2.69615774e+06     7.66827377e+03     6.36794424e+01    -2.76093531e+01    -6.09630581e-05    -3.02759104e-04     1.67683038e-05     8.36103093e-01     1.35824087e-03    -2.16547125e-03    -5.48566389e-01    -9.99633913e-01     2.48367072e-02     1.07321292e-02    -9.99528301e-01    -1.96289449e-02     2.36194845e-02     3.67437667e-07    -2.17558585e-05     2.99318092e-05    -1.44270328e-01    -1.70746224e+00     5.43977985e-02 1 ",
"    5.00000000e-01     6.51819923e+04    -6.21850554e+06     2.69614351e+06     7.66823329e+03     6.76592885e+01    -2.93348861e+01    -5.82766150e-05    -2.80713131e-04     1.60888447e-05     8.36111115e-01     1.39484830e-03    -2.22868441e-03    -5.48553817e-01    -9.99633915e-01     2.48366158e-02     1.07320897e-02    -9.99525009e-01    -1.96213682e-02     2.37646304e-02     3.42242193e-07    -2.17551881e-05     2.99321852e-05    -1.59926312e-01    -1.75420374e+00     6.03247625e-02 1 ",
"    5.00000000e-01     6.90160983e+04    -6.21847071e+06     2.69612841e+06     7.66819034e+03     7.16391129e+01    -3.10604096e+01    -5.47901845e-05    -2.63147305e-04     1.51991950e-05     8.36118720e-01     1.42890184e-03    -2.28765106e-03    -5.48541895e-01    -9.99633918e-01     2.48365243e-02     1.07320503e-02    -9.99521925e-01    -1.96141509e-02     2.38999080e-02     3.16755377e-07    -2.17544685e-05     2.99325632e-05    -1.76083553e-01    -1.79184573e+00     6.64520853e-02 1 ",
"    5.00000000e-01     7.28501822e+04    -6.21843390e+06     2.69611245e+06     7.66814494e+03     7.56189144e+01    -3.27859232e+01    -5.13038516e-05    -2.45581431e-04     1.43096745e-05     8.36125853e-01     1.46073044e-03    -2.34282517e-03    -5.48530707e-01    -9.99633921e-01     2.48364329e-02     1.07320109e-02    -9.99519026e-01    -1.96073403e-02     2.40264409e-02     2.91011999e-07    -2.17536937e-05     2.99329465e-05    -1.92240794e-01    -1.82948771e+00     7.25794082e-02 1 ",
"    5.00000000e-01     7.66842428e+04    -6.21839509e+06     2.69609562e+06     7.66809709e+03     7.95986917e+01    -3.45114263e+01    -4.70034586e-05    -2.32591375e-04     1.32067945e-05     8.36132457e-01     1.49066989e-03    -2.39467050e-03    -5.48520336e-01    -9.99633923e-01     2.48363414e-02     1.07319714e-02    -9.99516284e-01    -1.96009838e-02     2.41453765e-02     2.65047430e-07    -2.17528577e-05     2.99333386e-05    -2.08901361e-01    -1.85783524e+00     7.89078482e-02 1 ",
"    5.00000000e-01     8.05182789e+04    -6.21835430e+06     2.69607794e+06     7.66804679e+03     8.35784435e+01    -3.62369184e+01    -4.19991339e-05    -2.23895103e-04     1.19197381e-05     8.36138426e-01     1.51936941e-03    -2.44408710e-03    -5.48510940e-01    -9.99633926e-01     2.48362500e-02     1.07319320e-02    -9.99513648e-01    -1.95951712e-02     2.42589385e-02     2.38930324e-07    -2.17519491e-05     2.99337466e-05    -2.25867586e-01    -1.87743893e+00     8.53654493e-02 1 ",
"    5.00000000e-01     8.43522892e+04    -6.21831152e+06     2.69605939e+06     7.66799403e+03     8.75581686e+01    -3.79623989e+01    -3.69944997e-05    -2.15198779e-04     1.06337661e-05     8.36143711e-01     1.54714229e-03    -2.49151136e-03    -5.48502592e-01    -9.99633929e-01     2.48361585e-02     1.07318926e-02    -9.99511093e-01    -1.95899431e-02     2.43682033e-02     2.12693787e-07    -2.17509626e-05     2.99341736e-05    -2.42833810e-01    -1.89704262e+00     9.18230504e-02 1 ",
"    5.00000000e-01     8.81862725e+04    -6.21826674e+06     2.69603997e+06     7.66793882e+03     9.15378657e+01    -3.96878672e+01    -3.14035010e-05    -2.10491366e-04     9.19467231e-06     8.36148270e-01     1.57427769e-03    -2.53734988e-03    -5.48495354e-01    -9.99633932e-01     2.48360670e-02     1.07318532e-02    -9.99508594e-01    -1.95853335e-02     2.44741705e-02     1.86368619e-07    -2.17498940e-05     2.99346222e-05    -2.59896216e-01    -1.90849806e+00     9.83337198e-02 1 ",
"    5.00000000e-01     9.20202276e+04    -6.21821998e+06     2.69601970e+06     7.66788115e+03     9.55175334e+01    -4.14133229e+01    -2.52209616e-05    -2.09841532e-04     7.60149893e-06     8.36152021e-01     1.60135898e-03    -2.58242289e-03    -5.48489347e-01    -9.99633934e-01     2.48359755e-02     1.07318138e-02    -9.99506102e-01    -1.95814111e-02     2.45788565e-02     1.60016728e-07    -2.17487345e-05     2.99350983e-05    -2.77042668e-01    -1.91166329e+00     1.04893279e-01 1 ",
"    5.00000000e-01     9.58541532e+04    -6.21817122e+06     2.69599856e+06     7.66782103e+03     9.94971706e+01    -4.31387653e+01    -1.90377603e-05    -2.09191642e-04     6.01021471e-06     8.36154922e-01     1.62868000e-03    -2.62714418e-03    -5.48484632e-01    -9.99633937e-01     2.48358840e-02     1.07317744e-02    -9.99503592e-01    -1.95782099e-02     2.46832781e-02     1.33669327e-07    -2.17474796e-05     2.99356048e-05    -2.94189120e-01    -1.91482852e+00     1.11452838e-01 1 ",
"    5.00000000e-01     9.96880482e+04    -6.21812048e+06     2.69597656e+06     7.66775846e+03     1.03476776e+02    -4.48641938e+01    -1.23731111e-05    -2.12304352e-04     4.29387488e-06     8.36156938e-01     1.65651137e-03    -2.67189872e-03    -5.48481259e-01    -9.99633940e-01     2.48357924e-02     1.07317350e-02    -9.99501039e-01    -1.95757568e-02     2.47883777e-02     1.07355378e-07    -2.17461259e-05     2.99361442e-05    -3.11227159e-01    -1.91028119e+00     1.17991445e-01 1 ",
"    5.00000000e-01     1.03521911e+05    -6.21806775e+06     2.69595370e+06     7.66769343e+03     1.07456348e+02    -4.65896081e+01    -5.32976761e-06    -2.18894783e-04     2.47949929e-06     8.36158008e-01     1.68537243e-03    -2.71742819e-03    -5.48479317e-01    -9.99633942e-01     2.48357009e-02     1.07316956e-02    -9.99498397e-01    -1.95741017e-02     2.48959686e-02     8.11302673e-08    -2.17446669e-05     2.99367211e-05    -3.27980891e-01    -1.89858052e+00     1.24445359e-01 1 ",
"    5.00000000e-01     1.07355741e+05    -6.21801303e+06     2.69592997e+06     7.66762595e+03     1.11435886e+02    -4.83150074e+01     1.71453891e-06    -2.25485156e-04     6.67674217e-07     8.36158104e-01     1.71551139e-03    -2.76408966e-03    -5.48478843e-01    -9.99633945e-01     2.48356093e-02     1.07316562e-02    -9.99495643e-01    -1.95732654e-02     2.50069218e-02     5.50206793e-08    -2.17430999e-05     2.99373375e-05    -3.44734624e-01    -1.88687985e+00     1.30899273e-01 1 ",
"    5.00000000e-01     1.11189537e+05    -6.21795631e+06     2.69590538e+06     7.66755602e+03     1.15415388e+02    -5.00403912e+01     9.13493032e-06    -2.35597047e-04    -1.24121940e-06     8.36157200e-01     1.74717934e-03    -2.81224485e-03    -5.48479878e-01    -9.99633948e-01     2.48355177e-02     1.07316169e-02    -9.99492755e-01    -1.95732682e-02     2.51221188e-02     2.90534766e-08    -2.17414222e-05     2.99379959e-05    -3.61179960e-01    -1.86793284e+00     1.37260177e-01 1 ",
"    5.00000000e-01     1.15023297e+05    -6.21789761e+06     2.69587993e+06     7.66748363e+03     1.19394854e+02    -5.17657591e+01     1.68230005e-05    -2.48926323e-04    -3.21873473e-06     8.36155247e-01     1.78085531e-03    -2.86258693e-03    -5.48482485e-01    -9.99633950e-01     2.48354261e-02     1.07315775e-02    -9.99489685e-01    -1.95741466e-02     2.52432467e-02     3.28013706e-09    -2.17396289e-05     2.99386999e-05    -3.77132758e-01    -1.84233687e+00     1.43461384e-01 1 ",
"    5.00000000e-01     1.18857020e+05    -6.21783692e+06     2.69585362e+06     7.66740879e+03     1.23374281e+02    -5.34911103e+01     2.45123041e-05    -2.62255538e-04    -5.19313372e-06     8.36152227e-01     1.81676654e-03    -2.91544781e-03    -5.48486694e-01    -9.99633953e-01     2.48353345e-02     1.07315381e-02    -9.99486413e-01    -1.95759145e-02     2.53711107e-02    -2.22748688e-08    -2.17377182e-05     2.99394515e-05    -3.93085555e-01    -1.81674091e+00     1.49662591e-01 1 ",
"    5.00000000e-01     1.22690705e+05    -6.21777424e+06     2.69582644e+06     7.66733149e+03     1.27353668e+02    -5.52164445e+01     3.23680148e-05    -2.78508020e-04    -7.20964779e-06     8.36148125e-01     1.85511737e-03    -2.97113044e-03    -5.48492519e-01    -9.99633956e-01     2.48352429e-02     1.07314988e-02    -9.99482917e-01    -1.95785793e-02     2.55064420e-02    -4.75894135e-08    -2.17356889e-05     2.99402520e-05    -4.08377689e-01    -1.78507504e+00     1.55643299e-01 1 ",
"    5.00000000e-01     1.26524351e+05    -6.21770956e+06     2.69579840e+06     7.66725174e+03     1.31333015e+02    -5.69417610e+01     4.03804170e-05    -2.97703791e-04    -9.26556022e-06     8.36142917e-01     1.89631834e-03    -3.03024251e-03    -5.48499994e-01    -9.99633959e-01     2.48351513e-02     1.07314594e-02    -9.99479154e-01    -1.95821583e-02     2.56507080e-02    -7.26189962e-08    -2.17335386e-05     2.99411040e-05    -4.22974580e-01    -1.74729321e+00     1.61391521e-01 1 ",
"    5.00000000e-01     1.30357957e+05    -6.21764290e+06     2.69576950e+06     7.66716954e+03     1.35312320e+02    -5.86670592e+01     4.83942706e-05    -3.16899501e-04    -1.13179390e-05     8.36136591e-01     1.94057483e-03    -3.09308929e-03    -5.48509133e-01    -9.99633961e-01     2.48350597e-02     1.07314201e-02    -9.99475104e-01    -1.95866580e-02     2.58046447e-02    -9.73413876e-08    -2.17312663e-05     2.99420089e-05    -4.37571471e-01    -1.70951139e+00     1.67139743e-01 1 ",
"    5.00000000e-01     1.34191520e+05    -6.21757425e+06     2.69573973e+06     7.66708489e+03     1.39291581e+02    -6.03923387e+01     5.64718462e-05    -3.38756974e-04    -1.33854448e-05     8.36129138e-01     1.98807045e-03    -3.15994823e-03    -5.48519942e-01    -9.99633964e-01     2.48349680e-02     1.07313807e-02    -9.99470743e-01    -1.95920795e-02     2.59689177e-02    -1.21736449e-07    -2.17288715e-05     2.99429678e-05    -4.51323257e-01    -1.66616939e+00     1.72601323e-01 1 ",
"    5.00000000e-01     1.38025041e+05    -6.21750361e+06     2.69570910e+06     7.66699778e+03     1.43270798e+02    -6.21175989e+01     6.45161583e-05    -3.62978923e-04    -1.54428150e-05     8.36120555e-01     2.03915024e-03    -3.23134437e-03    -5.48532422e-01    -9.99633967e-01     2.48348763e-02     1.07313414e-02    -9.99466034e-01    -1.95984221e-02     2.61447827e-02    -1.45766521e-07    -2.17263540e-05     2.99439819e-05    -4.64075036e-01    -1.61785449e+00     1.77720319e-01 1 ",
"    5.00000000e-01     1.41858518e+05    -6.21743098e+06     2.69567761e+06     7.66690821e+03     1.47249969e+02    -6.38428392e+01     7.25620913e-05    -3.87200814e-04    -1.74963724e-05     8.36110841e-01     2.09397479e-03    -3.30752582e-03    -5.48546568e-01    -9.99633969e-01     2.48347847e-02     1.07313021e-02    -9.99460954e-01    -1.96056808e-02     2.63328303e-02    -1.69413823e-07    -2.17237142e-05     2.99450518e-05    -4.76826816e-01    -1.56953959e+00     1.82839316e-01 1 ",
"    5.00000000e-01     1.45691949e+05    -6.21735636e+06     2.69564526e+06     7.66681620e+03     1.51229093e+02    -6.55680591e+01     8.05591029e-05    -4.13785731e-04    -1.95356208e-05     8.36099996e-01     2.15270419e-03    -3.38874082e-03    -5.48562374e-01    -9.99633972e-01     2.48346930e-02     1.07312628e-02    -9.99455483e-01    -1.96138495e-02     2.65336507e-02    -1.92660716e-07    -2.17209526e-05     2.99461782e-05    -4.88535828e-01    -1.51624833e+00     1.87600883e-01 1 ",
"    5.00000000e-01     1.49525333e+05    -6.21727975e+06     2.69561205e+06     7.66672173e+03     1.55208169e+02    -6.72932579e+01     8.84182423e-05    -4.42448817e-04    -2.15374570e-05     8.36088031e-01     2.21563757e-03    -3.47545705e-03    -5.48579818e-01    -9.99633975e-01     2.48346013e-02     1.07312234e-02    -9.99449582e-01    -1.96229143e-02     2.67483525e-02    -2.15474179e-07    -2.17180705e-05     2.99473611e-05    -4.99064757e-01    -1.45854496e+00     1.91955449e-01 1 ",
"    5.00000000e-01     1.53358670e+05    -6.21720115e+06     2.69557797e+06     7.66662481e+03     1.59187195e+02    -6.90184353e+01     9.62791390e-05    -4.71111849e-04    -2.35353025e-05     8.36074951e-01     2.28291309e-03    -3.56789453e-03    -5.48598884e-01    -9.99633977e-01     2.48345095e-02     1.07311841e-02    -9.99443229e-01    -1.96328634e-02     2.69774540e-02    -2.37838716e-07    -2.17150690e-05     2.99486009e-05    -5.09593685e-01    -1.40084158e+00     1.96310016e-01 1 ",
"    5.00000000e-01     1.57191958e+05    -6.21712056e+06     2.69554303e+06     7.66652543e+03     1.63166170e+02    -7.07435905e+01     1.03921000e-04    -5.01580590e-04    -2.54747184e-05     8.36060768e-01     2.35464811e-03    -3.66624620e-03    -5.48619547e-01    -9.99633980e-01     2.48344178e-02     1.07311448e-02    -9.99436402e-01    -1.96436803e-02     2.72214054e-02    -2.59741127e-07    -2.17119499e-05     2.99498975e-05    -5.18822096e-01    -1.33926726e+00     2.00214139e-01 1 ",
"    5.00000000e-01     1.61025195e+05    -6.21703799e+06     2.69550723e+06     7.66642360e+03     1.67145093e+02    -7.24687232e+01     1.11323335e-04    -5.33834378e-04    -2.73503596e-05     8.36045509e-01     2.43107626e-03    -3.77089560e-03    -5.48641756e-01    -9.99633983e-01     2.48343261e-02     1.07311055e-02    -9.99429065e-01    -1.96553349e-02     2.74811011e-02    -2.81155022e-07    -2.17087166e-05     2.99512494e-05    -5.26701280e-01    -1.27385687e+00     2.03650868e-01 1 ",
"    5.00000000e-01     1.64858381e+05    -6.21695342e+06     2.69547056e+06     7.66631932e+03     1.71123962e+02    -7.41938326e+01     1.18727515e-04    -5.66088119e-04    -2.92219647e-05     8.36029189e-01     2.51231293e-03    -3.88203383e-03    -5.48665485e-01    -9.99633986e-01     2.48342343e-02     1.07310662e-02    -9.99421196e-01    -1.96678091e-02     2.77569859e-02    -3.02067380e-07    -2.17053710e-05     2.99526566e-05    -5.34580464e-01    -1.20844648e+00     2.07087596e-01 1 ",
"    5.00000000e-01     1.68691514e+05    -6.21686686e+06     2.69543303e+06     7.66621258e+03     1.75102776e+02    -7.59189183e+01     1.25808526e-04    -5.99842056e-04    -3.10081794e-05     8.36011824e-01     2.59845179e-03    -3.99982371e-03    -5.48690696e-01    -9.99633988e-01     2.48341425e-02     1.07310270e-02    -9.99412770e-01    -1.96810796e-02     2.80494328e-02    -3.22467281e-07    -2.17019155e-05     2.99541183e-05    -5.40987892e-01    -1.13976622e+00     2.10012721e-01 1 ",
"    5.00000000e-01     1.72524593e+05    -6.21677832e+06     2.69539464e+06     7.66610339e+03     1.79081535e+02    -7.76439797e+01     1.32491479e-04    -6.34825920e-04    -3.26897264e-05     8.35993459e-01     2.68966058e-03    -4.12456333e-03    -5.48717313e-01    -9.99633991e-01     2.48340507e-02     1.07309877e-02    -9.99403754e-01    -1.96951010e-02     2.83591199e-02    -3.42335245e-07    -2.16983555e-05     2.99556319e-05    -5.45819813e-01    -1.06835504e+00     2.12388819e-01 1 ",
"    5.00000000e-01     1.76357617e+05    -6.21668778e+06     2.69535539e+06     7.66599175e+03     1.83060236e+02    -7.93690162e+01     1.39176319e-04    -6.69809747e-04    -3.43673218e-05     8.35974119e-01     2.78601250e-03    -4.25638849e-03    -5.48745290e-01    -9.99633994e-01     2.48339589e-02     1.07309484e-02    -9.99394124e-01    -1.97098457e-02     2.86863524e-02    -3.61662586e-07    -2.16946941e-05     2.99571962e-05    -5.50651733e-01    -9.96943848e-01     2.14764917e-01 1 ",
"    5.00000000e-01     1.80190585e+05    -6.21659526e+06     2.69531527e+06     7.66587766e+03     1.87038879e+02    -8.10940274e+01     1.45438501e-04    -7.05985028e-04    -3.59339187e-05     8.35953827e-01     2.88757748e-03    -4.39543140e-03    -5.48774582e-01    -9.99633996e-01     2.48338671e-02     1.07309091e-02    -9.99383855e-01    -1.97252845e-02     2.90314253e-02    -3.80440826e-07    -2.16909345e-05     2.99588102e-05    -5.53854911e-01    -9.22872290e-01     2.16573446e-01 1 ",
"    5.00000000e-01     1.84023494e+05    -6.21650074e+06     2.69527429e+06     7.66576111e+03     1.91017461e+02    -8.28190126e+01     1.51211656e-04    -7.43096075e-04    -3.73725090e-05     8.35932641e-01     2.99447701e-03    -4.54193036e-03    -5.48805088e-01    -9.99633999e-01     2.48337753e-02     1.07308699e-02    -9.99372914e-01    -1.97413609e-02     2.93948653e-02    -3.98655296e-07    -2.16870834e-05     2.99604699e-05    -5.55343682e-01    -8.46651967e-01     2.17783476e-01 1 ",
};

/* for some reason, if the queue is created by the i42 app it works fine. if its created before running i42 app, some queue data seem to be messed up! */
int
CFE_PSP_SensorInit(void)
{
	int ret = OS_QueueCreate(&SensorQid, CFE_PSP_SENSORQ_NAME, CFE_PSP_SENSORQ_DEPTH, CFE_PSP_SENSOR_DATASZ, 0);

	if (ret != OS_SUCCESS) {
		if (ret == OS_ERR_NAME_TAKEN) {
			ret = OS_QueueGetIdByName(&SensorQid, CFE_PSP_SENSORQ_NAME);
		}

		if (ret != OS_SUCCESS) {
			OS_printf("Failed to create queue for sensor data transfer\n");
		}
	}
	assert(ret == OS_SUCCESS);

	OS_printf("Sensor Queue ID: %u\n", SensorQid);

	OS_printf("Sensor interrupt thread will be created later in the flow\n");

	return 0;
}

void
CFE_PSP_SensorISR(arcvcap_t rcv, void *p)
{
	unsigned int trace_curr = 0, trace_sz = sizeof(SensorTraceDump) / sizeof(SensorTraceDump[0]);
	cycles_t interval = sl_usec2cyc(CFE_PSP_SENSOR_INTERVAL_USEC);

	CFE_PSP_SensorInit();

	while (1) {
		int pending;
		cycles_t now, abs_timeout;
		char data[CFE_PSP_SENSOR_DATASZ] = { 0 };
		char time[32] = { 0 };

#ifndef SENSOREMU_USE_HPET
                now = sl_now();
                abs_timeout = now + interval;
                sl_thd_block_timeout(0, abs_timeout);

#else
                pending = cos_rcv(rcv, 0, NULL);
                assert(pending >= 0);
#endif
		strcpy(data, SensorTraceDump[trace_curr]);
		/* for RTT measurements */
		sprintf(time, "%llu\n", sl_now());
		strcat(data, time);
		//PRINTC("%s\n", data);
		/* write to queue */
		if (OS_QueuePut(SensorQid, (void *)data, strlen(data), 0) == OS_QUEUE_FULL) {
			OS_printf("Sensor queue full!\n");
		}

		trace_curr++;
		if (trace_curr >= trace_sz) trace_curr = 0;
	}

	pthread_exit(NULL);
}


