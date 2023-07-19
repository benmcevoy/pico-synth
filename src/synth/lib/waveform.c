#include <math.h>

#include "../include/waveform.h"
#include "../include/audiocontext.h"

#define WAVE_TABLE_LENGTH 512

float TriangleWaveTable[WAVE_TABLE_LENGTH] = {0.000000, 0.007812, 0.015625, 0.023438, 0.031250, 0.039062, 0.046875, 0.054688, 0.062500, 0.070312, 0.078125, 0.085938, 0.093750, 0.101562, 0.109375, 0.117188, 0.125000, 0.132812, 0.140625, 0.148438, 0.156250, 0.164062, 0.171875, 0.179688, 0.187500, 0.195312, 0.203125, 0.210938, 0.218750, 0.226562, 0.234375, 0.242188, 0.250000, 0.257812, 0.265625, 0.273438, 0.281250, 0.289062, 0.296875, 0.304688, 0.312500, 0.320312, 0.328125, 0.335938, 0.343750, 0.351562, 0.359375, 0.367188, 0.375000, 0.382812, 0.390625, 0.398438, 0.406250, 0.414062, 0.421875, 0.429688, 0.437500, 0.445312, 0.453125, 0.460938, 0.468750, 0.476562, 0.484375, 0.492188, 0.500000, 0.507812, 0.515625, 0.523438, 0.531250, 0.539062, 0.546875, 0.554688, 0.562500, 0.570312, 0.578125, 0.585938, 0.593750, 0.601562, 0.609375, 0.617188, 0.625000, 0.632812, 0.640625, 0.648438, 0.656250, 0.664062, 0.671875, 0.679688, 0.687500, 0.695312, 0.703125, 0.710938, 0.718750, 0.726562, 0.734375, 0.742188, 0.750000, 0.757812, 0.765625, 0.773438, 0.781250, 0.789062, 0.796875, 0.804688, 0.812500, 0.820312, 0.828125, 0.835938, 0.843750, 0.851562, 0.859375, 0.867188, 0.875000, 0.882812, 0.890625, 0.898438, 0.906250, 0.914062, 0.921875, 0.929688, 0.937500, 0.945312, 0.953125, 0.960938, 0.968750, 0.976562, 0.984375, 0.992188, 1.000000, 0.992188, 0.984375, 0.976562, 0.968750, 0.960938, 0.953125, 0.945312, 0.937500, 0.929688, 0.921875, 0.914062, 0.906250, 0.898438, 0.890625, 0.882812, 0.875000, 0.867188, 0.859375, 0.851562, 0.843750, 0.835938, 0.828125, 0.820312, 0.812500, 0.804688, 0.796875, 0.789062, 0.781250, 0.773438, 0.765625, 0.757812, 0.750000, 0.742188, 0.734375, 0.726562, 0.718750, 0.710938, 0.703125, 0.695312, 0.687500, 0.679688, 0.671875, 0.664062, 0.656250, 0.648438, 0.640625, 0.632812, 0.625000, 0.617188, 0.609375, 0.601562, 0.593750, 0.585938, 0.578125, 0.570312, 0.562500, 0.554688, 0.546875, 0.539062, 0.531250, 0.523438, 0.515625, 0.507812, 0.500000, 0.492188, 0.484375, 0.476562, 0.468750, 0.460938, 0.453125, 0.445312, 0.437500, 0.429688, 0.421875, 0.414062, 0.406250, 0.398438, 0.390625, 0.382812, 0.375000, 0.367188, 0.359375, 0.351562, 0.343750, 0.335938, 0.328125, 0.320312, 0.312500, 0.304688, 0.296875, 0.289062, 0.281250, 0.273438, 0.265625, 0.257812, 0.250000, 0.242188, 0.234375, 0.226562, 0.218750, 0.210938, 0.203125, 0.195312, 0.187500, 0.179688, 0.171875, 0.164062, 0.156250, 0.148438, 0.140625, 0.132812, 0.125000, 0.117188, 0.109375, 0.101562, 0.093750, 0.085938, 0.078125, 0.070312, 0.062500, 0.054688, 0.046875, 0.039062, 0.031250, 0.023438, 0.015625, 0.007812, 0.000000, -0.007812, -0.015625, -0.023438, -0.031250, -0.039062, -0.046875, -0.054688, -0.062500, -0.070312, -0.078125, -0.085938, -0.093750, -0.101562, -0.109375, -0.117188, -0.125000, -0.132812, -0.140625, -0.148438, -0.156250, -0.164062, -0.171875, -0.179688, -0.187500, -0.195312, -0.203125, -0.210938, -0.218750, -0.226562, -0.234375, -0.242188, -0.250000, -0.257812, -0.265625, -0.273438, -0.281250, -0.289062, -0.296875, -0.304688, -0.312500, -0.320312, -0.328125, -0.335938, -0.343750, -0.351562, -0.359375, -0.367188, -0.375000, -0.382812, -0.390625, -0.398438, -0.406250, -0.414062, -0.421875, -0.429688, -0.437500, -0.445312, -0.453125, -0.460938, -0.468750, -0.476562, -0.484375, -0.492188, -0.500000, -0.507812, -0.515625, -0.523438, -0.531250, -0.539062, -0.546875, -0.554688, -0.562500, -0.570312, -0.578125, -0.585938, -0.593750, -0.601562, -0.609375, -0.617188, -0.625000, -0.632812, -0.640625, -0.648438, -0.656250, -0.664062, -0.671875, -0.679688, -0.687500, -0.695312, -0.703125, -0.710938, -0.718750, -0.726562, -0.734375, -0.742188, -0.750000, -0.757812, -0.765625, -0.773438, -0.781250, -0.789062, -0.796875, -0.804688, -0.812500, -0.820312, -0.828125, -0.835938, -0.843750, -0.851562, -0.859375, -0.867188, -0.875000, -0.882812, -0.890625, -0.898438, -0.906250, -0.914062, -0.921875, -0.929688, -0.937500, -0.945312, -0.953125, -0.960938, -0.968750, -0.976562, -0.984375, -0.992188, -1.000000, -0.992188, -0.984375, -0.976562, -0.968750, -0.960938, -0.953125, -0.945312, -0.937500, -0.929688, -0.921875, -0.914062, -0.906250, -0.898438, -0.890625, -0.882812, -0.875000, -0.867188, -0.859375, -0.851562, -0.843750, -0.835938, -0.828125, -0.820312, -0.812500, -0.804688, -0.796875, -0.789062, -0.781250, -0.773438, -0.765625, -0.757812, -0.750000, -0.742188, -0.734375, -0.726562, -0.718750, -0.710938, -0.703125, -0.695312, -0.687500, -0.679688, -0.671875, -0.664062, -0.656250, -0.648438, -0.640625, -0.632812, -0.625000, -0.617188, -0.609375, -0.601562, -0.593750, -0.585938, -0.578125, -0.570312, -0.562500, -0.554688, -0.546875, -0.539062, -0.531250, -0.523438, -0.515625, -0.507812, -0.500000, -0.492188, -0.484375, -0.476562, -0.468750, -0.460938, -0.453125, -0.445312, -0.437500, -0.429688, -0.421875, -0.414062, -0.406250, -0.398438, -0.390625, -0.382812, -0.375000, -0.367188, -0.359375, -0.351562, -0.343750, -0.335938, -0.328125, -0.320312, -0.312500, -0.304688, -0.296875, -0.289062, -0.281250, -0.273438, -0.265625, -0.257812, -0.250000, -0.242188, -0.234375, -0.226562, -0.218750, -0.210938, -0.203125, -0.195312, -0.187500, -0.179688, -0.171875, -0.164062, -0.156250, -0.148438, -0.140625, -0.132812, -0.125000, -0.117188, -0.109375, -0.101562, -0.093750, -0.085938, -0.078125, -0.070312, -0.062500, -0.054688, -0.046875, -0.039062, -0.031250, -0.023438, -0.015625, -0.007812};
float SineWaveTable[WAVE_TABLE_LENGTH] = {0.000000, 0.012272, 0.024541, 0.036807, 0.049068, 0.061321, 0.073565, 0.085797, 0.098017, 0.110222, 0.122411, 0.134581, 0.146730, 0.158858, 0.170962, 0.183040, 0.195090, 0.207111, 0.219101, 0.231058, 0.242980, 0.254866, 0.266713, 0.278520, 0.290285, 0.302006, 0.313682, 0.325310, 0.336890, 0.348419, 0.359895, 0.371317, 0.382683, 0.393992, 0.405241, 0.416430, 0.427555, 0.438616, 0.449611, 0.460539, 0.471397, 0.482184, 0.492898, 0.503538, 0.514103, 0.524590, 0.534998, 0.545325, 0.555570, 0.565732, 0.575808, 0.585798, 0.595699, 0.605511, 0.615232, 0.624859, 0.634393, 0.643832, 0.653173, 0.662416, 0.671559, 0.680601, 0.689541, 0.698376, 0.707107, 0.715731, 0.724247, 0.732654, 0.740951, 0.749136, 0.757209, 0.765167, 0.773010, 0.780737, 0.788346, 0.795837, 0.803208, 0.810457, 0.817585, 0.824589, 0.831470, 0.838225, 0.844854, 0.851355, 0.857729, 0.863973, 0.870087, 0.876070, 0.881921, 0.887640, 0.893224, 0.898674, 0.903989, 0.909168, 0.914210, 0.919114, 0.923880, 0.928506, 0.932993, 0.937339, 0.941544, 0.945607, 0.949528, 0.953306, 0.956940, 0.960431, 0.963776, 0.966976, 0.970031, 0.972940, 0.975702, 0.978317, 0.980785, 0.983105, 0.985278, 0.987301, 0.989177, 0.990903, 0.992480, 0.993907, 0.995185, 0.996313, 0.997290, 0.998118, 0.998795, 0.999322, 0.999699, 0.999925, 1.000000, 0.999925, 0.999699, 0.999322, 0.998795, 0.998118, 0.997290, 0.996313, 0.995185, 0.993907, 0.992480, 0.990903, 0.989177, 0.987301, 0.985278, 0.983105, 0.980785, 0.978317, 0.975702, 0.972940, 0.970031, 0.966976, 0.963776, 0.960431, 0.956940, 0.953306, 0.949528, 0.945607, 0.941544, 0.937339, 0.932993, 0.928506, 0.923880, 0.919114, 0.914210, 0.909168, 0.903989, 0.898674, 0.893224, 0.887640, 0.881921, 0.876070, 0.870087, 0.863973, 0.857729, 0.851355, 0.844854, 0.838225, 0.831470, 0.824589, 0.817585, 0.810457, 0.803208, 0.795837, 0.788346, 0.780737, 0.773010, 0.765167, 0.757209, 0.749136, 0.740951, 0.732654, 0.724247, 0.715731, 0.707107, 0.698376, 0.689541, 0.680601, 0.671559, 0.662416, 0.653173, 0.643832, 0.634393, 0.624859, 0.615232, 0.605511, 0.595699, 0.585798, 0.575808, 0.565732, 0.555570, 0.545325, 0.534998, 0.524590, 0.514103, 0.503538, 0.492898, 0.482184, 0.471397, 0.460539, 0.449611, 0.438616, 0.427555, 0.416430, 0.405241, 0.393992, 0.382683, 0.371317, 0.359895, 0.348419, 0.336890, 0.325310, 0.313682, 0.302006, 0.290285, 0.278520, 0.266713, 0.254866, 0.242980, 0.231058, 0.219101, 0.207111, 0.195090, 0.183040, 0.170962, 0.158858, 0.146730, 0.134581, 0.122411, 0.110222, 0.098017, 0.085797, 0.073565, 0.061321, 0.049068, 0.036807, 0.024541, 0.012272, 0.000000, -0.012272, -0.024541, -0.036807, -0.049068, -0.061321, -0.073565, -0.085797, -0.098017, -0.110222, -0.122411, -0.134581, -0.146730, -0.158858, -0.170962, -0.183040, -0.195090, -0.207111, -0.219101, -0.231058, -0.242980, -0.254866, -0.266713, -0.278520, -0.290285, -0.302006, -0.313682, -0.325310, -0.336890, -0.348419, -0.359895, -0.371317, -0.382683, -0.393992, -0.405241, -0.416430, -0.427555, -0.438616, -0.449611, -0.460539, -0.471397, -0.482184, -0.492898, -0.503538, -0.514103, -0.524590, -0.534998, -0.545325, -0.555570, -0.565732, -0.575808, -0.585798, -0.595699, -0.605511, -0.615232, -0.624859, -0.634393, -0.643832, -0.653173, -0.662416, -0.671559, -0.680601, -0.689541, -0.698376, -0.707107, -0.715731, -0.724247, -0.732654, -0.740951, -0.749136, -0.757209, -0.765167, -0.773010, -0.780737, -0.788346, -0.795837, -0.803208, -0.810457, -0.817585, -0.824589, -0.831470, -0.838225, -0.844854, -0.851355, -0.857729, -0.863973, -0.870087, -0.876070, -0.881921, -0.887640, -0.893224, -0.898674, -0.903989, -0.909168, -0.914210, -0.919114, -0.923880, -0.928506, -0.932993, -0.937339, -0.941544, -0.945607, -0.949528, -0.953306, -0.956940, -0.960431, -0.963776, -0.966976, -0.970031, -0.972940, -0.975702, -0.978317, -0.980785, -0.983105, -0.985278, -0.987301, -0.989177, -0.990903, -0.992480, -0.993907, -0.995185, -0.996313, -0.997290, -0.998118, -0.998795, -0.999322, -0.999699, -0.999925, -1.000000, -0.999925, -0.999699, -0.999322, -0.998795, -0.998118, -0.997290, -0.996313, -0.995185, -0.993907, -0.992480, -0.990903, -0.989177, -0.987301, -0.985278, -0.983105, -0.980785, -0.978317, -0.975702, -0.972940, -0.970031, -0.966976, -0.963776, -0.960431, -0.956940, -0.953306, -0.949528, -0.945607, -0.941544, -0.937339, -0.932993, -0.928506, -0.923880, -0.919114, -0.914210, -0.909168, -0.903989, -0.898674, -0.893224, -0.887640, -0.881921, -0.876070, -0.870087, -0.863973, -0.857729, -0.851355, -0.844854, -0.838225, -0.831470, -0.824589, -0.817585, -0.810457, -0.803208, -0.795837, -0.788346, -0.780737, -0.773010, -0.765167, -0.757209, -0.749136, -0.740951, -0.732654, -0.724247, -0.715731, -0.707107, -0.698376, -0.689541, -0.680601, -0.671559, -0.662416, -0.653173, -0.643832, -0.634393, -0.624859, -0.615232, -0.605511, -0.595699, -0.585798, -0.575808, -0.565732, -0.555570, -0.545325, -0.534998, -0.524590, -0.514103, -0.503538, -0.492898, -0.482184, -0.471397, -0.460539, -0.449611, -0.438616, -0.427555, -0.416430, -0.405241, -0.393992, -0.382683, -0.371317, -0.359895, -0.348419, -0.336890, -0.325310, -0.313682, -0.302006, -0.290285, -0.278520, -0.266713, -0.254866, -0.242980, -0.231058, -0.219101, -0.207111, -0.195090, -0.183040, -0.170962, -0.158858, -0.146730, -0.134581, -0.122411, -0.110222, -0.098017, -0.085797, -0.073565, -0.061321, -0.049068, -0.036807, -0.024541, -0.012272};
float SawWaveTable[WAVE_TABLE_LENGTH] = {0.000000, 0.023675, 0.047313, 0.070878, 0.094333, 0.117644, 0.140773, 0.163688, 0.186353, 0.208735, 0.230802, 0.252524, 0.273869, 0.294810, 0.315319, 0.335370, 0.354938, 0.374001, 0.392537, 0.410526, 0.427952, 0.444797, 0.461048, 0.476692, 0.491718, 0.506118, 0.519884, 0.533011, 0.545497, 0.557339, 0.568537, 0.579094, 0.589013, 0.598299, 0.606958, 0.615000, 0.622432, 0.629267, 0.635516, 0.641193, 0.646312, 0.650887, 0.654936, 0.658476, 0.661524, 0.664098, 0.666217, 0.667901, 0.669169, 0.670040, 0.670535, 0.670674, 0.670477, 0.669962, 0.669151, 0.668061, 0.666713, 0.665124, 0.663312, 0.661295, 0.659089, 0.656712, 0.654177, 0.651500, 0.648695, 0.645775, 0.642753, 0.639639, 0.636445, 0.633180, 0.629853, 0.626473, 0.623047, 0.619581, 0.616081, 0.612553, 0.609000, 0.605427, 0.601836, 0.598229, 0.594608, 0.590975, 0.587330, 0.583673, 0.580005, 0.576325, 0.572631, 0.568924, 0.565201, 0.561462, 0.557705, 0.553927, 0.550129, 0.546307, 0.542461, 0.538588, 0.534689, 0.530760, 0.526801, 0.522812, 0.518792, 0.514740, 0.510655, 0.506539, 0.502392, 0.498213, 0.494005, 0.489768, 0.485503, 0.481213, 0.476899, 0.472563, 0.468209, 0.463838, 0.459453, 0.455057, 0.450654, 0.446247, 0.441838, 0.437432, 0.433031, 0.428640, 0.424261, 0.419899, 0.415555, 0.411234, 0.406939, 0.402673, 0.398438, 0.394237, 0.390072, 0.385946, 0.381861, 0.377819, 0.373821, 0.369867, 0.365960, 0.362100, 0.358286, 0.354520, 0.350802, 0.347129, 0.343503, 0.339922, 0.336384, 0.332889, 0.329434, 0.326018, 0.322639, 0.319294, 0.315981, 0.312698, 0.309442, 0.306210, 0.303000, 0.299809, 0.296633, 0.293471, 0.290320, 0.287176, 0.284038, 0.280903, 0.277768, 0.274631, 0.271491, 0.268344, 0.265191, 0.262028, 0.258855, 0.255670, 0.252474, 0.249264, 0.246041, 0.242805, 0.239555, 0.236292, 0.233016, 0.229728, 0.226429, 0.223119, 0.219801, 0.216476, 0.213145, 0.209810, 0.206473, 0.203135, 0.199800, 0.196468, 0.193143, 0.189826, 0.186519, 0.183225, 0.179945, 0.176683, 0.173439, 0.170216, 0.167015, 0.163839, 0.160688, 0.157564, 0.154468, 0.151402, 0.148365, 0.145360, 0.142385, 0.139442, 0.136530, 0.133649, 0.130799, 0.127980, 0.125189, 0.122427, 0.119692, 0.116983, 0.114299, 0.111637, 0.108995, 0.106373, 0.103767, 0.101176, 0.098598, 0.096029, 0.093469, 0.090914, 0.088362, 0.085812, 0.083259, 0.080703, 0.078142, 0.075572, 0.072993, 0.070402, 0.067798, 0.065179, 0.062544, 0.059892, 0.057220, 0.054530, 0.051819, 0.049088, 0.046337, 0.043564, 0.040771, 0.037957, 0.035124, 0.032272, 0.029401, 0.026514, 0.023610, 0.020692, 0.017761, 0.014819, 0.011867, 0.008907, 0.005942, 0.002972, 0.000000, -0.002972, -0.005942, -0.008907, -0.011867, -0.014819, -0.017761, -0.020692, -0.023610, -0.026514, -0.029401, -0.032272, -0.035124, -0.037957, -0.040771, -0.043564, -0.046337, -0.049088, -0.051819, -0.054530, -0.057220, -0.059892, -0.062544, -0.065179, -0.067798, -0.070402, -0.072993, -0.075572, -0.078142, -0.080703, -0.083259, -0.085812, -0.088362, -0.090914, -0.093469, -0.096029, -0.098598, -0.101176, -0.103767, -0.106373, -0.108995, -0.111637, -0.114299, -0.116983, -0.119692, -0.122427, -0.125189, -0.127980, -0.130799, -0.133649, -0.136530, -0.139442, -0.142385, -0.145360, -0.148365, -0.151402, -0.154468, -0.157564, -0.160688, -0.163839, -0.167015, -0.170216, -0.173439, -0.176683, -0.179945, -0.183225, -0.186519, -0.189826, -0.193143, -0.196468, -0.199800, -0.203135, -0.206473, -0.209810, -0.213145, -0.216476, -0.219801, -0.223119, -0.226429, -0.229728, -0.233016, -0.236292, -0.239555, -0.242805, -0.246041, -0.249264, -0.252474, -0.255670, -0.258855, -0.262028, -0.265191, -0.268344, -0.271491, -0.274631, -0.277768, -0.280903, -0.284038, -0.287176, -0.290320, -0.293471, -0.296633, -0.299809, -0.303000, -0.306210, -0.309442, -0.312698, -0.315981, -0.319294, -0.322639, -0.326018, -0.329434, -0.332889, -0.336384, -0.339922, -0.343503, -0.347129, -0.350802, -0.354520, -0.358286, -0.362100, -0.365960, -0.369867, -0.373821, -0.377819, -0.381861, -0.385946, -0.390072, -0.394237, -0.398437, -0.402673, -0.406939, -0.411234, -0.415555, -0.419899, -0.424261, -0.428640, -0.433031, -0.437432, -0.441838, -0.446247, -0.450654, -0.455057, -0.459453, -0.463838, -0.468209, -0.472563, -0.476899, -0.481213, -0.485503, -0.489768, -0.494005, -0.498213, -0.502392, -0.506539, -0.510655, -0.514740, -0.518792, -0.522812, -0.526801, -0.530760, -0.534689, -0.538588, -0.542461, -0.546307, -0.550129, -0.553927, -0.557705, -0.561462, -0.565201, -0.568924, -0.572631, -0.576325, -0.580005, -0.583673, -0.587330, -0.590975, -0.594608, -0.598229, -0.601836, -0.605427, -0.609000, -0.612553, -0.616081, -0.619581, -0.623047, -0.626473, -0.629853, -0.633180, -0.636445, -0.639639, -0.642753, -0.645775, -0.648695, -0.651500, -0.654177, -0.656712, -0.659089, -0.661295, -0.663312, -0.665124, -0.666713, -0.668061, -0.669151, -0.669962, -0.670477, -0.670674, -0.670535, -0.670040, -0.669169, -0.667901, -0.666217, -0.664098, -0.661524, -0.658476, -0.654936, -0.650887, -0.646312, -0.641193, -0.635516, -0.629267, -0.622432, -0.615000, -0.606958, -0.598299, -0.589013, -0.579094, -0.568537, -0.557339, -0.545497, -0.533011, -0.519884, -0.506118, -0.491718, -0.476692, -0.461048, -0.444797, -0.427952, -0.410526, -0.392537, -0.374001, -0.354938, -0.335370, -0.315319, -0.294810, -0.273869, -0.252524, -0.230802, -0.208735, -0.186353, -0.163688, -0.140773, -0.117644, -0.094333, -0.070878, -0.047313, -0.023675};

float _readPointer = 0;
float _phase = 0;

float read_from_wt(AudioContext_t *context, float *waveTable)
{
    uint indexBelow = floorf(_readPointer);
    uint indexAbove = indexBelow + 1;

    if (indexAbove >= WAVE_TABLE_LENGTH)
        indexAbove -= WAVE_TABLE_LENGTH;

    float fractionAbove = _readPointer - indexBelow;
    float fractionBelow = 1.f - fractionAbove;

    float value = fractionBelow * waveTable[indexBelow] + fractionAbove * waveTable[indexAbove];

    _readPointer += WAVE_TABLE_LENGTH * context->Voice.frequency / context->SampleRate;

    while (_readPointer >= WAVE_TABLE_LENGTH)
        _readPointer -= WAVE_TABLE_LENGTH;

    return value;
}

float square(AudioContext_t *context)
{
    float value = (_phase < M_PI) ? 1.0 : -1.0;
    
    _phase += TWOPI * context->Voice.frequency / context->SampleRate;

    if (_phase > TWOPI) _phase -= TWOPI;

    return value;
}

float noise(AudioContext_t *context)
{
    // random between -1..1
    return (rand() / (float)(RAND_MAX)) * 2.f - 1.f;
}

float synth_waveform_sample(AudioContext_t *context)
{
    Waveform_t waveform = context->Voice.waveform;

    if (waveform == SINE)
        return read_from_wt(context, SineWaveTable);

    if (waveform == SQUARE)
        return square(context);

    if (waveform == SAW)
        return read_from_wt(context, SawWaveTable);

    if (waveform == TRIANGLE)
        return read_from_wt(context, TriangleWaveTable);

    if (waveform == NOISE)
        return noise(context);

    return 0.f;
}