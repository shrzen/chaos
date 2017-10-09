# include "hosttab.h"
struct host_entry host_table[];
struct net_entry net_table[];
struct host_data everything = {
	net_table,
	host_table,
	&host_table[304],
	45,
	439
};
struct net_entry net_table[] = {
	{"amprnet", 44, 0},
	{"arpanet", 10, 0},
	{"autodin-ii", 26, 0},
	{"bbn-ln-test", 41, 0},
	{"bbn-local", 24, 0},
	{"bbn-pr", 1, 0},
	{"bbn-rcc", 3, 0},
	{"bbn-sat-test", 31, 0},
	{"bragg-pr", 9, 0},
	{"chaos", 7, 1},
	{"cislnet", 40, 0},
	{"clarknet", 8, 0},
	{"cyclades", 12, 0},
	{"datapac", 16, 0},
	{"dc-pr", 20, 0},
	{"dcn-comsat", 29, 0},
	{"dcn-ucl", 30, 0},
	{"decnet", 37, 0},
	{"decnet-test", 38, 0},
	{"dialnet", 22, 0},
	{"dsk", 511, 0},
	{"edn", 21, 0},
	{"epss", 15, 0},
	{"intelpost", 43, 0},
	{"lcsnet", 18, 0},
	{"matnet", 34, 0},
	{"mitre", 23, 0},
	{"nosc-lccn", 27, 0},
	{"null", 35, 0},
	{"ru-net", 49, 0},
	{"rsre-ppsn", 25, 0},
	{"s1net", 42, 0},
	{"satnet", 4, 0},
	{"sf-pr-1", 2, 0},
	{"sf-pr-2", 6, 0},
	{"sill-pr", 5, 0},
	{"srinet", 39, 0},
	{"su-net", 36, 0},
	{"telenet", 14, 0},
	{"transpac", 17, 0},
	{"tymnet", 19, 0},
	{"ucl-cr1", 32, 0},
	{"ucl-cr2", 33, 0},
	{"uclnet", 11, 0},
	{"wideband", 28, 0},
};
static char h_s0[] = "unix";
static char h_s1[] = "tip";
static char h_s2[] = "vms";
static char h_s3[] = "nos/be";
static char h_s4[] = "tac";
static char h_s5[] = "rsx11m";
static char h_s6[] = "tops-20";
static char h_s7[] = "scope";
static char h_s8[] = "tss/360";
static char h_s9[] = "os-mvt";
static char h_s10[] = "dms";
static char h_s11[] = "elf";
static char h_s12[] = "";
static char h_s13[] = "ncc";
static char h_s14[] = "mtip";
static char h_s15[] = "tenex";
static char h_s16[] = "mpx-rt";
static char h_s17[] = "mos";
static char h_s18[] = "mp-32a";
static char h_s19[] = "tops-10";
static char h_s20[] = "rt11";
static char h_s21[] = "multics";
static char h_s22[] = "bky";
static char h_s23[] = "vm-370";
static char h_s24[] = "its";
static char h_s25[] = "chaos-gateway";
static char h_s26[] = "magicsix";
static char h_s27[] = "lispm";
static char h_s28[] = "minits";
static char h_s29[] = "plasma";
static char h_s30[] = "rsx";
static char h_s31[] = "dos";
static char h_s32[] = "tftpsp";
static char h_s33[] = "pli";
static char h_s34[] = "kronos";
static char h_s35[] = "exec-8";
static char h_s36[] = "rsx11";
static char h_s37[] = "rsx-11m";
static char h_s38[] = "nos";
static char h_s39[] = "rig";
static char h_s40[] = "waits";
static char h_s41[] = "nswit";
static char h_s42[] = "dos/360";
static char h_s43[] = "ddt";
static char h_s44[] = "alto";
static char h_s45[] = "os/vs";
static char h_s46[] = "ifs";
static char h_s47[] = "dolphin";
static char h_s48[] = "sunet-gateway";
static char h_s49[] = "sun";
static char h_s50[] = "sun-tip";
static char h_s51[] = "dover";
static char h_s52[] = "ethertip";
static char h_s53[] = "ants";
static char h_m0[] = "pdp11";
static char h_m1[] = "h316";
static char h_m2[] = "vax";
static char h_m3[] = "cdc-6600";
static char h_m4[] = "pdp10";
static char h_m5[] = "ibm-360/67";
static char h_m6[] = "ibm-370/195";
static char h_m7[] = "pdp15";
static char h_m8[] = "c70";
static char h_m9[] = "";
static char h_m10[] = "pluribus";
static char h_m11[] = "cdc-7600";
static char h_m12[] = "cdc-5600";
static char h_m13[] = "ap90";
static char h_m14[] = "cdc-6500";
static char h_m15[] = "h6180";
static char h_m16[] = "ibm-370/168";
static char h_m17[] = "ibm-370";
static char h_m18[] = "pe3230";
static char h_m19[] = "lispm";
static char h_m20[] = "h6880";
static char h_m21[] = "h68dps";
static char h_m22[] = "alto";
static char h_m23[] = "univac-1110";
static char h_m24[] = "eclipse-s200";
static char h_m25[] = "ibm-360/44";
static char h_m26[] = "ibm-360/40";
static char h_m27[] = "ibm-3081";
static char h_m28[] = "dolphin";
static char h_m29[] = "sun-gateway";
static char h_m30[] = "sun";
static char h_m31[] = "ibm-360/91";
static char h_m32[] = "amdahl";
static struct net_address addr0[] = {
	012, 0266,
	0, 0
};
static struct net_address addr1[] = {
	012, 0243,
	0, 0
};
static char *nic1[] = {
	"nelc-tip",
	0
};
static struct net_address addr2[] = {
	012, 064,
	0, 0
};
static char *nic2[] = {
	"isi-vaxb",
	"vaxb",
	"ajpo",
	0
};
static struct net_address addr3[] = {
	012, 0301,
	0, 0
};
static char *nic3[] = {
	"a",
	0
};
static struct net_address addr4[] = {
	012, 0202,
	0, 0
};
static struct net_address addr5[] = {
	012, 0302,
	0, 0
};
static struct net_address addr6[] = {
	012, 065,
	0, 0
};
static char *nic6[] = {
	"eglin",
	0
};
static struct net_address addr7[] = {
	012, 0265,
	0, 0
};
static char *nic7[] = {
	"eglin-dev",
	0
};
static struct net_address addr8[] = {
	012, 0103,
	0, 0
};
static char *nic8[] = {
	"hqafsc",
	0
};
static struct net_address addr9[] = {
	012, 0203,
	0, 0
};
static char *nic9[] = {
	"hqafsc-tip",
	0
};
static struct net_address addr10[] = {
	012, 0101,
	0, 0
};
static char *nic10[] = {
	"afsd",
	0
};
static struct net_address addr11[] = {
	012, 0201,
	0, 0
};
static char *nic11[] = {
	"sd-tip",
	0
};
static struct net_address addr12[] = {
	012, 060,
	0, 0
};
static char *nic12[] = {
	"awful",
	0
};
static struct net_address addr13[] = {
	012, 0260,
	0, 0
};
static char *nic13[] = {
	"awful-tip",
	0
};
static struct net_address addr14[] = {
	012, 0320,
	0, 0
};
static struct net_address addr15[] = {
	012, 020,
	0, 0
};
static char *nic15[] = {
	"ames",
	0
};
static struct net_address addr16[] = {
	012, 0220,
	0, 0
};
static struct net_address addr17[] = {
	012, 067,
	0, 0
};
static char *nic17[] = {
	"argonne",
	0
};
static struct net_address addr18[] = {
	012, 034,
	0, 0
};
static struct net_address addr19[] = {
	012, 0334,
	0, 0
};
static char *nic19[] = {
	"arpa-xgp11",
	"arpa-png11",
	0
};
static struct net_address addr20[] = {
	012, 0234,
	0, 0
};
static struct net_address addr21[] = {
	03, 06,
	0, 0
};
static char *nic21[] = {
	"bbnadmin",
	"admin",
	0
};
static struct net_address addr22[] = {
	012, 0307,
	0, 0
};
static char *nic22[] = {
	"clxx",
	0
};
static struct net_address addr23[] = {
	012, 0350,
	0, 0
};
static struct net_address addr24[] = {
	012, 0410,
	0, 0
};
static struct net_address addr25[] = {
	012, 0177,
	0, 0
};
static char *nic25[] = {
	"bbnn",
	0
};
static struct net_address addr26[] = {
	012, 050,
	0, 0
};
static char *nic26[] = {
	"ncc",
	0
};
static struct net_address addr27[] = {
	012, 0110,
	0, 0
};
static char *nic27[] = {
	"bbn-noc",
	"bbnnu",
	"nu",
	0
};
static struct net_address addr28[] = {
	012, 0205,
	03, 0205,
	0, 0
};
static char *nic28[] = {
	"ptip",
	0
};
static struct net_address addr29[] = {
	012, 077,
	0, 0
};
static char *nic29[] = {
	"bbnr",
	0
};
static struct net_address addr30[] = {
	012, 0261,
	0, 0
};
static struct net_address addr31[] = {
	012, 0310,
	0, 0
};
static struct net_address addr32[] = {
	012, 0236,
	0, 0
};
static struct net_address addr33[] = {
	012, 0210,
	03, 07,
	0, 0
};
static char *nic33[] = {
	"bbnu",
	0
};
static struct net_address addr34[] = {
	012, 0222,
	0, 0
};
static char *nic34[] = {
	"bbnv",
	0
};
static struct net_address addr35[] = {
	012, 0305,
	03, 0305,
	0, 0
};
static char *nic35[] = {
	"bbn-tenexa",
	0
};
static struct net_address addr36[] = {
	012, 061,
	03, 061,
	0, 0
};
static char *nic36[] = {
	"bbn-tenexb",
	0
};
static struct net_address addr37[] = {
	012, 0361,
	03, 0361,
	0, 0
};
static char *nic37[] = {
	"bbn-tenex",
	"bbn-tenexc",
	"bbn",
	"bbnd",
	"bbne",
	0
};
static struct net_address addr38[] = {
	03, 0363,
	0, 0
};
static char *nic38[] = {
	"bbncc",
	0
};
static struct net_address addr39[] = {
	03, 0102,
	0, 0
};
static struct net_address addr40[] = {
	012, 05,
	03, 05,
	0, 0
};
static char *nic40[] = {
	"bbn-tenexf",
	0
};
static struct net_address addr41[] = {
	012, 0105,
	03, 0105,
	0, 0
};
static char *nic41[] = {
	"bbn-tenexg",
	0
};
static struct net_address addr42[] = {
	012, 0422,
	0, 0
};
static struct net_address addr43[] = {
	03, 0207,
	0, 0
};
static struct net_address addr44[] = {
	012, 0277,
	0, 0
};
static struct net_address addr45[] = {
	012, 0122,
	0, 0
};
static struct net_address addr46[] = {
	03, 0306,
	0, 0
};
static struct net_address addr47[] = {
	012, 0172,
	0, 0
};
static char *nic47[] = {
	"brookhaven",
	0
};
static struct net_address addr48[] = {
	012, 046,
	0, 0
};
static struct net_address addr49[] = {
	012, 0146,
	0, 0
};
static struct net_address addr50[] = {
	012, 0246,
	0, 0
};
static struct net_address addr51[] = {
	012, 035,
	0, 0
};
static struct net_address addr52[] = {
	012, 0335,
	0, 0
};
static char *nic52[] = {
	"bmd70",
	0
};
static struct net_address addr53[] = {
	012, 0235,
	0, 0
};
static struct net_address addr54[] = {
	012, 0237,
	0, 0
};
static char *nic54[] = {
	"cca-tip",
	0
};
static struct net_address addr55[] = {
	012, 037,
	0, 0
};
static char *nic55[] = {
	"cca",
	"cca-tenex",
	"cca-sdms",
	0
};
static struct net_address addr56[] = {
	012, 0137,
	0, 0
};
static struct net_address addr57[] = {
	012, 024,
	0, 0
};
static struct net_address addr58[] = {
	012, 074,
	0, 0
};
static struct net_address addr59[] = {
	012, 0374,
	0, 0
};
static struct net_address addr60[] = {
	012, 0107,
	0, 0
};
static char *nic60[] = {
	"chii",
	0
};
static struct net_address addr61[] = {
	012, 0244,
	0, 0
};
static char *nic61[] = {
	"sixpac-tip",
	0
};
static struct net_address addr62[] = {
	012, 0144,
	0, 0
};
static struct net_address addr63[] = {
	012, 066,
	0, 0
};
static char *nic63[] = {
	"caltech",
	"cit",
	0
};
static struct net_address addr64[] = {
	012, 0166,
	0, 0
};
static char *nic64[] = {
	"cit-11",
	0
};
static struct net_address addr65[] = {
	012, 0207,
	0, 0
};
static struct net_address addr66[] = {
	012, 0116,
	0, 0
};
static char *nic66[] = {
	"cmua",
	"cmu-a",
	"cmu",
	0
};
static struct net_address addr67[] = {
	012, 016,
	0, 0
};
static char *nic67[] = {
	"cmub",
	"cmu-b",
	0
};
static struct net_address addr68[] = {
	012, 0316,
	0, 0
};
static char *nic68[] = {
	"cmuc",
	"cmu-c",
	"pi",
	0
};
static struct net_address addr69[] = {
	012, 0216,
	0, 0
};
static char *nic69[] = {
	"cmu-10d",
	"cmud",
	0
};
static struct net_address addr70[] = {
	012, 0171,
	0, 0
};
static char *nic70[] = {
	"coins",
	0
};
static struct net_address addr71[] = {
	012, 044,
	0, 0
};
static struct net_address addr72[] = {
	012, 056,
	0, 0
};
static struct net_address addr73[] = {
	012, 0256,
	0, 0
};
static struct net_address addr74[] = {
	012, 0321,
	0, 0
};
static struct net_address addr75[] = {
	012, 0274,
	0, 0
};
static struct net_address addr76[] = {
	012, 0245,
	0, 0
};
static char *nic76[] = {
	"purdue-tcp",
	"purdue-rvax",
	"pu-rvax",
	0
};
static struct net_address addr77[] = {
	012, 0236,
	0, 0
};
static char *nic77[] = {
	"csnetb",
	0
};
static struct net_address addr78[] = {
	012, 063,
	0, 0
};
static char *nic78[] = {
	"darcom",
	"sri-tenex",
	"sri-ka",
	"ka",
	0
};
static struct net_address addr79[] = {
	012, 0262,
	0, 0
};
static struct net_address addr80[] = {
	012, 0321,
	0, 0
};
static char *nic80[] = {
	"dtnsrdc-tip",
	0
};
static struct net_address addr81[] = {
	012, 0224,
	0, 0
};
static struct net_address addr82[] = {
	012, 0117,
	0, 0
};
static char *nic82[] = {
	"2136",
	"kl2136",
	0
};
static struct net_address addr83[] = {
	012, 0217,
	0, 0
};
static char *nic83[] = {
	"dec",
	"market",
	"2244",
	"kl2244",
	0
};
static struct net_address addr84[] = {
	012, 0114,
	0, 0
};
static struct net_address addr85[] = {
	012, 014,
	0, 0
};
static struct net_address addr86[] = {
	012, 0221,
	0, 0
};
static char *nic86[] = {
	"nsrdc",
	0
};
static struct net_address addr87[] = {
	012, 0324,
	0, 0
};
static struct net_address addr88[] = {
	012, 073,
	0, 0
};
static struct net_address addr89[] = {
	012, 0141,
	0, 0
};
static char *nic89[] = {
	"fnwc",
	"fenwick",
	0
};
static struct net_address addr90[] = {
	012, 0341,
	0, 0
};
static char *nic90[] = {
	"fnwc-secure",
	0
};
static struct net_address addr91[] = {
	012, 0115,
	0, 0
};
static char *nic91[] = {
	"adam",
	0
};
static struct net_address addr92[] = {
	012, 0215,
	0, 0
};
static char *nic92[] = {
	"gunt",
	0
};
static struct net_address addr93[] = {
	012, 015,
	0, 0
};
static char *nic93[] = {
	"gafs",
	0
};
static struct net_address addr94[] = {
	012, 011,
	0, 0
};
static char *nic94[] = {
	"acl",
	0
};
static struct net_address addr95[] = {
	026, 037777751200,
	0, 0
};
static char *nic95[] = {
	"hp",
	0
};
static struct net_address addr96[] = {
	012, 0120,
	0, 0
};
static char *nic96[] = {
	"honey",
	0
};
static struct net_address addr97[] = {
	012, 012464,
	0, 0
};
static struct net_address addr98[] = {
	012, 0133,
	0, 0
};
static char *nic98[] = {
	"isi-xgp11",
	0
};
static struct net_address addr99[] = {
	012, 0326,
	0, 0
};
static struct net_address addr100[] = {
	012, 026,
	0, 0
};
static struct net_address addr101[] = {
	012, 0233,
	0, 0
};
static char *nic101[] = {
	"vaxa",
	0
};
static struct net_address addr102[] = {
	012, 013464,
	0, 0
};
static struct net_address addr103[] = {
	012, 0366,
	0, 0
};
static struct net_address addr104[] = {
	012, 0340,
	0, 0
};
static char *nic104[] = {
	"sci",
	"sci-ics",
	"sct",
	"kes",
	0
};
static struct net_address addr105[] = {
	012, 042,
	0, 0
};
static struct net_address addr106[] = {
	012, 0142,
	0, 0
};
static struct net_address addr107[] = {
	012, 012,
	0, 0
};
static struct net_address addr108[] = {
	012, 0312,
	0, 0
};
static struct net_address addr109[] = {
	012, 0154,
	0, 0
};
static struct net_address addr110[] = {
	012, 0112,
	0, 0
};
static struct net_address addr111[] = {
	012, 0212,
	0, 0
};
static struct net_address addr112[] = {
	012, 0125,
	0, 0
};
static char *nic112[] = {
	"mfe",
	0
};
static struct net_address addr113[] = {
	012, 025,
	0, 0
};
static char *nic113[] = {
	"lll",
	"lll-comp",
	0
};
static struct net_address addr114[] = {
	012, 0203,
	0, 0
};
static struct net_address addr115[] = {
	012, 052,
	0, 0
};
static char *nic115[] = {
	"ukics-370",
	"uk-ics",
	0
};
static struct net_address addr116[] = {
	012, 0352,
	0, 0
};
static char *nic116[] = {
	"satnet",
	0
};
static struct net_address addr117[] = {
	012, 0252,
	0, 0
};
static struct net_address addr118[] = {
	012, 0365,
	0, 0
};
static struct net_address addr119[] = {
	012, 0120,
	0, 0
};
static struct net_address addr120[] = {
	012, 0206,
	07, 02026,
	0, 0
};
static char *nic120[] = {
	"mit-ai-10",
	"mit-ai-ka",
	"ai",
	"ai10",
	"mitai",
	0
};
static struct net_address addr121[] = {
	07, 0426,
	07, 03072,
	07, 01006,
	0, 0
};
static char *nic121[] = {
	"ai-chaos-11",
	0
};
static struct net_address addr122[] = {
	022, 05100,
	0, 0
};
static char *nic122[] = {
	"ajax",
	0
};
static struct net_address addr123[] = {
	07, 03570,
	0, 0
};
static char *nic123[] = {
	"alcvax",
	"alcator",
	0
};
static struct net_address addr124[] = {
	07, 016300,
	0, 0
};
static char *nic124[] = {
	"devo",
	"arcmac",
	"mit-amg",
	"amg",
	"arcmac-devo",
	0
};
static struct net_address addr125[] = {
	07, 024364,
	0, 0
};
static char *nic125[] = {
	"arcmac-nick",
	"nick",
	0
};
static struct net_address addr126[] = {
	07, 016134,
	0, 0
};
static char *nic126[] = {
	"arthur",
	"mit-lispm-11",
	"cadr-11",
	"cadr11",
	"lm11",
	0
};
static struct net_address addr127[] = {
	07, 03420,
	0, 0
};
static char *nic127[] = {
	"benji",
	"mit-ee-admin",
	0
};
static struct net_address addr128[] = {
	022, 04005,
	022, 05005,
	022, 01007,
	07, 0420,
	0, 0
};
static char *nic128[] = {
	"bridge",
	0
};
static struct net_address addr129[] = {
	07, 0530,
	07, 03430,
	07, 07430,
	07, 016130,
	07, 011100,
	0, 0
};
static char *nic129[] = {
	"bypass",
	0
};
static struct net_address addr130[] = {
	07, 016310,
	0, 0
};
static char *nic130[] = {
	"mitccc",
	"ccc",
	0
};
static struct net_address addr131[] = {
	07, 016260,
	0, 0
};
static char *nic131[] = {
	"cipg",
	0
};
static struct net_address addr132[] = {
	07, 016340,
	0, 0
};
static char *nic132[] = {
	"mit-cogs",
	"cogs",
	0
};
static struct net_address addr133[] = {
	022, 05012,
	0, 0
};
static char *nic133[] = {
	"csg",
	0
};
static struct net_address addr134[] = {
	022, 05010,
	0, 0
};
static char *nic134[] = {
	"csr",
	"mit-ln",
	"ln",
	0
};
static struct net_address addr135[] = {
	012, 0337,
	0, 0
};
static char *nic135[] = {
	"cisl",
	"devmultics",
	0
};
static struct net_address addr136[] = {
	012, 0106,
	0, 0
};
static char *nic136[] = {
	"dm",
	"dms",
	"mitdm",
	"mit-dm",
	0
};
static struct net_address addr137[] = {
	07, 016240,
	0, 0
};
static char *nic137[] = {
	"dspg",
	0
};
static struct net_address addr138[] = {
	07, 03404,
	0, 0
};
static char *nic138[] = {
	"eddie",
	0
};
static struct net_address addr139[] = {
	07, 05542,
	0, 0
};
static char *nic139[] = {
	"ee",
	"eecs",
	"mitee",
	"mit-ee",
	"mit-deep-thought",
	"deep-thought",
	0
};
static struct net_address addr140[] = {
	07, 0542,
	0, 0
};
static char *nic140[] = {
	"ee-network-11",
	"roosta",
	0
};
static struct net_address addr141[] = {
	07, 0510,
	0, 0
};
static char *nic141[] = {
	"eetest",
	0
};
static struct net_address addr142[] = {
	07, 016150,
	0, 0
};
static char *nic142[] = {
	"ford",
	"mit-lispm-14",
	"cadr-14",
	"cadr14",
	"lm14",
	0
};
static struct net_address addr143[] = {
	07, 03414,
	0, 0
};
static char *nic143[] = {
	"franky",
	"mit-ee-tv",
	0
};
static struct net_address addr144[] = {
	07, 03601,
	0, 0
};
static char *nic144[] = {
	"fusion",
	"pfc",
	"pfc-11",
	0
};
static struct net_address addr145[] = {
	012, 0115,
	022, 04004,
	0, 0
};
static char *nic145[] = {
	"gw",
	"mit-gateway",
	0
};
static struct net_address addr146[] = {
	07, 03067,
	0, 0
};
static char *nic146[] = {
	"htjr",
	"htbroke",
	0
};
static struct net_address addr147[] = {
	07, 03025,
	0, 0
};
static char *nic147[] = {
	"ht",
	"htvax",
	"htunix",
	0
};
static struct net_address addr148[] = {
	07, 010700,
	0, 0
};
static char *nic148[] = {
	"jcf-11",
	0
};
static struct net_address addr149[] = {
	07, 03050,
	0, 0
};
static char *nic149[] = {
	"cadr-1",
	"cadr1",
	"lm1",
	0
};
static struct net_address addr150[] = {
	07, 03140,
	0, 0
};
static char *nic150[] = {
	"cadr-2",
	"cadr2",
	"lm2",
	0
};
static struct net_address addr151[] = {
	07, 03034,
	0, 0
};
static char *nic151[] = {
	"cadr-3",
	"cadr3",
	"lm3",
	0
};
static struct net_address addr152[] = {
	07, 03062,
	0, 0
};
static char *nic152[] = {
	"cadr-4",
	"cadr4",
	"lm4",
	0
};
static struct net_address addr153[] = {
	07, 03036,
	0, 0
};
static char *nic153[] = {
	"cadr-5",
	"cadr5",
	"lm5",
	0
};
static struct net_address addr154[] = {
	07, 03064,
	0, 0
};
static char *nic154[] = {
	"cadr-6",
	"cadr6",
	"lm6",
	0
};
static struct net_address addr155[] = {
	07, 03022,
	0, 0
};
static char *nic155[] = {
	"cadr-7",
	"cadr7",
	"lm7",
	0
};
static struct net_address addr156[] = {
	07, 03142,
	0, 0
};
static char *nic156[] = {
	"cadr-8",
	"cadr8",
	"lm8",
	0
};
static struct net_address addr157[] = {
	07, 03074,
	0, 0
};
static char *nic157[] = {
	"cadr-9",
	"cadr9",
	"lm9",
	0
};
static struct net_address addr158[] = {
	07, 016250,
	0, 0
};
static char *nic158[] = {
	"cadr-12",
	"cadr12",
	"lm12",
	0
};
static struct net_address addr159[] = {
	07, 03066,
	0, 0
};
static char *nic159[] = {
	"cadr-15",
	"cadr15",
	"lm15",
	0
};
static struct net_address addr160[] = {
	07, 03070,
	0, 0
};
static char *nic160[] = {
	"cadr-16",
	"cadr16",
	"lm16",
	0
};
static struct net_address addr161[] = {
	07, 03076,
	0, 0
};
static char *nic161[] = {
	"cadr-18",
	"cadr18",
	"lm18",
	0
};
static struct net_address addr162[] = {
	07, 03100,
	0, 0
};
static char *nic162[] = {
	"cadr-19",
	"cadr19",
	"lm19",
	0
};
static struct net_address addr163[] = {
	07, 03102,
	0, 0
};
static char *nic163[] = {
	"cadr-20",
	"cadr20",
	"lm20",
	0
};
static struct net_address addr164[] = {
	07, 03104,
	0, 0
};
static char *nic164[] = {
	"cadr-22",
	"cadr22",
	"lm22",
	0
};
static struct net_address addr165[] = {
	07, 03042,
	0, 0
};
static char *nic165[] = {
	"cadr-23",
	"cadr23",
	"lm23",
	0
};
static struct net_address addr166[] = {
	07, 03044,
	0, 0
};
static char *nic166[] = {
	"cadr-24",
	"cadr24",
	"lm24",
	0
};
static struct net_address addr167[] = {
	07, 03110,
	0, 0
};
static char *nic167[] = {
	"cadr-25",
	"cadr25",
	"lm25",
	0
};
static struct net_address addr168[] = {
	07, 016210,
	0, 0
};
static char *nic168[] = {
	"cadr-26",
	"cadr26",
	"lm26",
	0
};
static struct net_address addr169[] = {
	07, 03060,
	0, 0
};
static char *nic169[] = {
	"cadr-27",
	"cadr27",
	"lm27",
	"mit-ai-filecomputer",
	"fc",
	"fs",
	0
};
static struct net_address addr170[] = {
	07, 03063,
	0, 0
};
static char *nic170[] = {
	"cadr-29",
	"cadr29",
	"lm29",
	0
};
static struct net_address addr171[] = {
	07, 03024,
	0, 0
};
static char *nic171[] = {
	"cadr-test",
	0
};
static struct net_address addr172[] = {
	07, 07500,
	0, 0
};
static char *nic172[] = {
	"lns",
	"lnsvax1",
	"mit-lnsvax-1",
	0
};
static struct net_address addr173[] = {
	07, 07510,
	0, 0
};
static char *nic173[] = {
	"lnsvax2",
	0
};
static struct net_address addr174[] = {
	07, 016136,
	0, 0
};
static char *nic174[] = {
	"marvin",
	"mit-lispm-13",
	"cadr-13",
	"cadr13",
	"lm13",
	0
};
static struct net_address addr175[] = {
	07, 07770,
	0, 0
};
static char *nic175[] = {
	"math",
	"mitmath",
	0
};
static struct net_address addr176[] = {
	012, 0354,
	07, 01440,
	0, 0
};
static char *nic176[] = {
	"mc",
	"mitmc",
	0
};
static struct net_address addr177[] = {
	07, 0440,
	07, 03040,
	0, 0
};
static char *nic177[] = {
	"mc-io-11",
	0
};
static struct net_address addr178[] = {
	012, 0306,
	07, 03114,
	0, 0
};
static char *nic178[] = {
	"ml",
	"mitml",
	0
};
static struct net_address addr179[] = {
	07, 03032,
	0, 0
};
static struct net_address addr180[] = {
	012, 06,
	0, 0
};
static char *nic180[] = {
	"multics",
	0
};
static struct net_address addr181[] = {
	07, 016050,
	0, 0
};
static char *nic181[] = {
	"cmultics",
	"cmul",
	0
};
static struct net_address addr182[] = {
	07, 03502,
	0, 0
};
static char *nic182[] = {
	"nplasma",
	0
};
static struct net_address addr183[] = {
	07, 011406,
	0, 0
};
static char *nic183[] = {
	"oz",
	"pumpkin",
	0
};
static struct net_address addr184[] = {
	07, 0406,
	07, 011446,
	0, 0
};
static char *nic184[] = {
	"oz-11",
	"oz-chaos-11",
	"toto",
	0
};
static struct net_address addr185[] = {
	07, 03106,
	0, 0
};
static char *nic185[] = {
	"oz-tip-1",
	"oztip1",
	0
};
static struct net_address addr186[] = {
	07, 03144,
	0, 0
};
static char *nic186[] = {
	"oz-tip-2",
	"oztip2",
	0
};
static struct net_address addr187[] = {
	07, 03621,
	0, 0
};
static char *nic187[] = {
	"pfc-test",
	"pfctest",
	0
};
static struct net_address addr188[] = {
	07, 03631,
	0, 0
};
static char *nic188[] = {
	"pfcvax",
	"pfc-vax",
	0
};
static struct net_address addr189[] = {
	07, 03641,
	0, 0
};
static char *nic189[] = {
	"pfcv80",
	"pfc-v80",
	0
};
static struct net_address addr190[] = {
	07, 0500,
	07, 03500,
	07, 016100,
	0, 0
};
static char *nic190[] = {
	"plasma",
	0
};
static struct net_address addr191[] = {
	07, 0470,
	0, 0
};
static char *nic191[] = {
	"rts",
	"dssr",
	0
};
static struct net_address addr192[] = {
	07, 03122,
	0, 0
};
static char *nic192[] = {
	"rts-tip-1",
	"rtip1",
	0
};
static struct net_address addr193[] = {
	07, 03124,
	0, 0
};
static char *nic193[] = {
	"rts-tip-2",
	"rtip2",
	0
};
static struct net_address addr194[] = {
	07, 03126,
	0, 0
};
static char *nic194[] = {
	"rts-tip-3",
	"rtip3",
	0
};
static struct net_address addr195[] = {
	07, 03130,
	0, 0
};
static char *nic195[] = {
	"rts-tip-4",
	"rtip4",
	0
};
static struct net_address addr196[] = {
	07, 03132,
	0, 0
};
static char *nic196[] = {
	"rts-tip-5",
	"rtip5",
	0
};
static struct net_address addr197[] = {
	07, 0402,
	07, 020005,
	0, 0
};
static char *nic197[] = {
	"dasher",
	0
};
static struct net_address addr198[] = {
	07, 016020,
	0, 0
};
static char *nic198[] = {
	"mitsipb",
	"sipb",
	0
};
static struct net_address addr199[] = {
	07, 012035,
	0, 0
};
static char *nic199[] = {
	"speech",
	"nspeech",
	0
};
static struct net_address addr200[] = {
	07, 0435,
	0, 0
};
static char *nic200[] = {
	"speech-11",
	"chatter",
	0
};
static struct net_address addr201[] = {
	022, 01200,
	0, 0
};
static char *nic201[] = {
	"spooler",
	0
};
static struct net_address addr202[] = {
	07, 011300,
	0, 0
};
static char *nic202[] = {
	"stucen",
	0
};
static struct net_address addr203[] = {
	012, 0315,
	0, 0
};
static struct net_address addr204[] = {
	022, 04050,
	022, 01017,
	022, 04600,
	07, 0410,
	0, 0
};
static char *nic204[] = {
	"mit-tiu1",
	"tiu",
	"tiu1",
	0
};
static struct net_address addr205[] = {
	012, 0254,
	022, 04051,
	0, 0
};
static char *nic205[] = {
	"mit-tgw",
	"tgw",
	0
};
static struct net_address addr206[] = {
	07, 03120,
	0, 0
};
static char *nic206[] = {
	"vx",
	"mit-vx",
	0
};
static struct net_address addr207[] = {
	012, 054,
	07, 02420,
	0, 0
};
static char *nic207[] = {
	"xx",
	"mitxx",
	0
};
static struct net_address addr208[] = {
	07, 03020,
	022, 04040,
	0, 0
};
static char *nic208[] = {
	"xx-network-11",
	"xi",
	0
};
static struct net_address addr209[] = {
	07, 016132,
	0, 0
};
static char *nic209[] = {
	"zaphod",
	"mit-lispm-10",
	"cadr-10",
	"cadr10",
	"lm10",
	0
};
static struct net_address addr210[] = {
	07, 016152,
	0, 0
};
static char *nic210[] = {
	"zarniwoop",
	"mit-lispm-17",
	"cadr-17",
	"cadr17",
	"lm17",
	0
};
static struct net_address addr211[] = {
	012, 021,
	0, 0
};
static struct net_address addr212[] = {
	012, 0102,
	0, 0
};
static struct net_address addr213[] = {
	012, 0221,
	0, 0
};
static struct net_address addr214[] = {
	012, 055,
	0, 0
};
static char *nic214[] = {
	"moffett",
	"moffett-arc",
	"arc-a",
	0
};
static struct net_address addr215[] = {
	012, 0155,
	0, 0
};
static char *nic215[] = {
	"moffett-subnet",
	"arc-b",
	0
};
static struct net_address addr216[] = {
	012, 0230,
	0, 0
};
static struct net_address addr217[] = {
	012, 0121,
	0, 0
};
static char *nic217[] = {
	"nalcon-devel",
	0
};
static struct net_address addr218[] = {
	012, 0323,
	0, 0
};
static char *nic218[] = {
	"nbs-unix",
	0
};
static struct net_address addr219[] = {
	012, 0123,
	0, 0
};
static struct net_address addr220[] = {
	012, 0223,
	0, 0
};
static struct net_address addr221[] = {
	012, 023,
	0, 0
};
static char *nic221[] = {
	"nbs",
	0
};
static struct net_address addr222[] = {
	012, 0250,
	0, 0
};
static struct net_address addr223[] = {
	012, 0165,
	0, 0
};
static char *nic223[] = {
	"ncsl",
	0
};
static struct net_address addr224[] = {
	012, 0130,
	0, 0
};
static char *nic224[] = {
	"nlm",
	"mcs",
	0
};
static struct net_address addr225[] = {
	012, 03,
	0, 0
};
static char *nic225[] = {
	"nuc-cc",
	"nosc-elf",
	"nosc",
	0
};
static struct net_address addr226[] = {
	012, 0143,
	0, 0
};
static char *nic226[] = {
	"nelc-elf",
	"nelc",
	"sdl",
	0
};
static struct net_address addr227[] = {
	012, 043,
	0, 0
};
static char *nic227[] = {
	"usc-isir1",
	"isir1",
	0
};
static struct net_address addr228[] = {
	012, 0343,
	0, 0
};
static struct net_address addr229[] = {
	012, 0103,
	0, 0
};
static char *nic229[] = {
	"nosc-secure1",
	0
};
static struct net_address addr230[] = {
	012, 0303,
	0, 0
};
static char *nic230[] = {
	"nprdc-unix",
	"nprdc-11",
	"nprdc-atts",
	0
};
static struct net_address addr231[] = {
	012, 041,
	0, 0
};
static struct net_address addr232[] = {
	012, 0241,
	0, 0
};
static struct net_address addr233[] = {
	012, 010,
	0, 0
};
static struct net_address addr234[] = {
	012, 0610,
	0, 0
};
static struct net_address addr235[] = {
	012, 0710,
	0, 0
};
static char *nic235[] = {
	"css",
	"nrl-csd",
	0
};
static struct net_address addr236[] = {
	012, 01010,
	0, 0
};
static struct net_address addr237[] = {
	012, 0124,
	0, 0
};
static struct net_address addr238[] = {
	012, 0324,
	0, 0
};
static struct net_address addr239[] = {
	012, 0210,
	0, 0
};
static struct net_address addr240[] = {
	012, 0372,
	0, 0
};
static struct net_address addr241[] = {
	012, 0334,
	0, 0
};
static char *nic241[] = {
	"npt",
	0
};
static struct net_address addr242[] = {
	012, 0125,
	0, 0
};
static struct net_address addr243[] = {
	012, 0425,
	0, 0
};
static struct net_address addr244[] = {
	012, 0225,
	0, 0
};
static char *nic244[] = {
	"nwc",
	0
};
static struct net_address addr245[] = {
	012, 0325,
	0, 0
};
static struct net_address addr246[] = {
	012, 072,
	0, 0
};
static struct net_address addr247[] = {
	012, 053,
	0, 0
};
static char *nic247[] = {
	"of1",
	0
};
static struct net_address addr248[] = {
	012, 0153,
	0, 0
};
static char *nic248[] = {
	"of2",
	"office",
	"off",
	0
};
static struct net_address addr249[] = {
	012, 0253,
	0, 0
};
static char *nic249[] = {
	"of3",
	"almsa",
	0
};
static struct net_address addr250[] = {
	012, 0353,
	0, 0
};
static char *nic250[] = {
	"of7",
	"cecom",
	0
};
static struct net_address addr251[] = {
	012, 0135,
	0, 0
};
static char *nic251[] = {
	"of8",
	0
};
static struct net_address addr252[] = {
	012, 0356,
	0, 0
};
static struct net_address addr253[] = {
	012, 040,
	0, 0
};
static char *nic253[] = {
	"parc",
	"maxc",
	"xerox",
	0
};
static struct net_address addr254[] = {
	012, 0332,
	0, 0
};
static struct net_address addr255[] = {
	012, 0232,
	0, 0
};
static struct net_address addr256[] = {
	03, 01,
	0, 0
};
static struct net_address addr257[] = {
	012, 045,
	0, 0
};
static char *nic257[] = {
	"purdue-cs",
	"pu-cs",
	"pu-unix",
	0
};
static struct net_address addr258[] = {
	012, 022,
	0, 0
};
static char *nic258[] = {
	"radc",
	0
};
static struct net_address addr259[] = {
	012, 0222,
	0, 0
};
static char *nic259[] = {
	"radt",
	0
};
static struct net_address addr260[] = {
	012, 0322,
	0, 0
};
static char *nic260[] = {
	"radc-20",
	0
};
static struct net_address addr261[] = {
	012, 0522,
	0, 0
};
static struct net_address addr262[] = {
	012, 0122,
	0, 0
};
static char *nic262[] = {
	"xper",
	0
};
static struct net_address addr263[] = {
	012, 07,
	0, 0
};
static char *nic263[] = {
	"rand-rcc",
	"rail",
	"bland",
	"wcc",
	0
};
static struct net_address addr264[] = {
	012, 0333,
	0, 0
};
static struct net_address addr265[] = {
	012, 0207,
	0, 0
};
static struct net_address addr266[] = {
	012, 0307,
	0, 0
};
static char *nic266[] = {
	"rand-isd",
	"isd",
	0
};
static struct net_address addr267[] = {
	012, 0400,
	0, 0
};
static struct net_address addr268[] = {
	012, 0422,
	0, 0
};
static char *nic268[] = {
	"roch",
	0
};
static struct net_address addr269[] = {
	061, 02,
	0, 0
};
static char *nic269[] = {
	"green",
	0
};
static struct net_address addr270[] = {
	012, 0272,
	061, 01,
	0, 0
};
static char *nic270[] = {
	"ru-ai",
	"rutgers-10",
	"rutgers-20",
	"red",
	"ru-red",
	0
};
static struct net_address addr271[] = {
	012, 0237,
	0, 0
};
static char *nic271[] = {
	"s-1",
	"s1",
	"s1-coprolite",
	0
};
static struct net_address addr272[] = {
	012, 0337,
	0, 0
};
static struct net_address addr273[] = {
	012, 0437,
	0, 0
};
static struct net_address addr274[] = {
	012, 0137,
	07, 010404,
	0, 0
};
static struct net_address addr275[] = {
	07, 010403,
	0, 0
};
static struct net_address addr276[] = {
	07, 010400,
	0, 0
};
static struct net_address addr277[] = {
	07, 010405,
	0, 0
};
static struct net_address addr278[] = {
	012, 0320,
	0, 0
};
static struct net_address addr279[] = {
	012, 0127,
	0, 0
};
static char *nic279[] = {
	"snl",
	0
};
static struct net_address addr280[] = {
	07, 021001,
	0, 0
};
static char *nic280[] = {
	"donner",
	0
};
static struct net_address addr281[] = {
	07, 017044,
	0, 0
};
static char *nic281[] = {
	"afghan",
	0
};
static struct net_address addr282[] = {
	07, 017002,
	0, 0
};
static char *nic282[] = {
	"basset",
	0
};
static struct net_address addr283[] = {
	07, 017003,
	0, 0
};
static char *nic283[] = {
	"beagle",
	0
};
static struct net_address addr284[] = {
	07, 017012,
	0, 0
};
static char *nic284[] = {
	"borzoi",
	0
};
static struct net_address addr285[] = {
	07, 017016,
	0, 0
};
static char *nic285[] = {
	"boxer",
	0
};
static struct net_address addr286[] = {
	07, 017026,
	0, 0
};
static char *nic286[] = {
	"bulldog",
	"bull",
	"dog",
	0
};
static struct net_address addr287[] = {
	07, 017010,
	0, 0
};
static char *nic287[] = {
	"collie",
	0
};
static struct net_address addr288[] = {
	07, 017020,
	0, 0
};
static char *nic288[] = {
	"comet",
	0
};
static struct net_address addr289[] = {
	07, 017040,
	0, 0
};
static char *nic289[] = {
	"dachshund",
	"hund",
	0
};
static struct net_address addr290[] = {
	07, 017042,
	0, 0
};
static char *nic290[] = {
	"dalmatian",
	"spot",
	0
};
static struct net_address addr291[] = {
	07, 017036,
	07, 024410,
	07, 020401,
	0, 0
};
static char *nic291[] = {
	"dancer",
	0
};
static struct net_address addr292[] = {
	07, 017022,
	0, 0
};
static char *nic292[] = {
	"husky",
	0
};
static struct net_address addr293[] = {
	07, 017006,
	0, 0
};
static char *nic293[] = {
	"pointer",
	"ptr",
	0
};
static struct net_address addr294[] = {
	07, 017024,
	07, 020001,
	07, 024430,
	0, 0
};
static char *nic294[] = {
	"prancer",
	0
};
static struct net_address addr295[] = {
	07, 017007,
	0, 0
};
static char *nic295[] = {
	"retriever",
	0
};
static struct net_address addr296[] = {
	07, 017034,
	0, 0
};
static char *nic296[] = {
	"samoyed",
	0
};
static struct net_address addr297[] = {
	07, 017200,
	0, 0
};
static char *nic297[] = {
	"schnauzer",
	0
};
static struct net_address addr298[] = {
	07, 017014,
	0, 0
};
static char *nic298[] = {
	"setter",
	0
};
static struct net_address addr299[] = {
	07, 017032,
	0, 0
};
static char *nic299[] = {
	"shepherd",
	0
};
static struct net_address addr300[] = {
	07, 017004,
	0, 0
};
static char *nic300[] = {
	"spaniel",
	0
};
static struct net_address addr301[] = {
	07, 017402,
	0, 0
};
static char *nic301[] = {
	"scrc",
	0
};
static struct net_address addr302[] = {
	07, 017005,
	0, 0
};
static char *nic302[] = {
	"terrier",
	0
};
static struct net_address addr303[] = {
	07, 017001,
	07, 017401,
	0, 0
};
static char *nic303[] = {
	"scrc-11",
	"rudolph",
	0
};
static struct net_address addr304[] = {
	07, 017050,
	0, 0
};
static char *nic304[] = {
	"vixen",
	0
};
static struct net_address addr305[] = {
	07, 023401,
	0, 0
};
static char *nic305[] = {
	"scrc-3600",
	"zippy",
	0
};
static struct net_address addr306[] = {
	07, 020402,
	07, 021002,
	0, 0
};
static char *nic306[] = {
	"blitzen",
	0
};
static struct net_address addr307[] = {
	012, 0347,
	0, 0
};
static char *nic307[] = {
	"ts44",
	"sdac-44",
	0
};
static struct net_address addr308[] = {
	012, 047,
	0, 0
};
static char *nic308[] = {
	"ccp",
	"sdac-ccp",
	0
};
static struct net_address addr309[] = {
	012, 0247,
	0, 0
};
static char *nic309[] = {
	"sdac-nep",
	0
};
static struct net_address addr310[] = {
	012, 0147,
	0, 0
};
static char *nic310[] = {
	"sdac",
	"src",
	"sdac-unix",
	0
};
static struct net_address addr311[] = {
	012, 0311,
	0, 0
};
static char *nic311[] = {
	"sri-20",
	"aic",
	0
};
static struct net_address addr312[] = {
	012, 0163,
	0, 0
};
static char *nic312[] = {
	"pkt40",
	"c3po",
	"c3p0",
	"sri-c3p0",
	"sri-tpm",
	"tpm",
	0
};
static struct net_address addr313[] = {
	012, 0202,
	0, 0
};
static char *nic313[] = {
	"sri-f2",
	"f2",
	"csl",
	"sri-vis11",
	0
};
static struct net_address addr314[] = {
	012, 0411,
	0, 0
};
static char *nic314[] = {
	"iuv",
	0
};
static struct net_address addr315[] = {
	012, 0102,
	0, 0
};
static char *nic315[] = {
	"sri",
	"kl",
	0
};
static struct net_address addr316[] = {
	012, 0111,
	0, 0
};
static char *nic316[] = {
	"nic",
	"sri-f3",
	0
};
static struct net_address addr317[] = {
	012, 02,
	0, 0
};
static char *nic317[] = {
	"dnsri",
	"nsc11",
	0
};
static struct net_address addr318[] = {
	012, 0363,
	0, 0
};
static char *nic318[] = {
	"pkt34",
	"r2d2",
	"sri-prmh",
	"prmh",
	0
};
static struct net_address addr319[] = {
	012, 0302,
	0, 0
};
static char *nic319[] = {
	"sri-tscb",
	"tscb",
	0
};
static struct net_address addr320[] = {
	012, 0263,
	0, 0
};
static char *nic320[] = {
	"thx",
	0
};
static struct net_address addr321[] = {
	012, 0211,
	0, 0
};
static char *nic321[] = {
	"warf",
	"tscf",
	0
};
static struct net_address addr322[] = {
	012, 0275,
	0, 0
};
static char *nic322[] = {
	"st-louis",
	"stl-tip",
	0
};
static struct net_address addr323[] = {
	044, 050,
	0, 0
};
static char *nic323[] = {
	"1145",
	0
};
static struct net_address addr324[] = {
	012, 013,
	044, 050,
	026, 042151,
	0, 0
};
static char *nic324[] = {
	"sail",
	"suai",
	0
};
static struct net_address addr325[] = {
	044, 050,
	0, 0
};
static char *nic325[] = {
	"almanor",
	0
};
static struct net_address addr326[] = {
	044, 050,
	0, 0
};
static char *nic326[] = {
	"cit1",
	0
};
static struct net_address addr327[] = {
	044, 050,
	0, 0
};
static char *nic327[] = {
	"cit2",
	0
};
static struct net_address addr328[] = {
	044, 050,
	0, 0
};
static char *nic328[] = {
	"cypress",
	0
};
static struct net_address addr329[] = {
	044, 050,
	0, 0
};
static char *nic329[] = {
	"diego",
	0
};
static struct net_address addr330[] = {
	012, 0170,
	044, 050,
	0, 0
};
static char *nic330[] = {
	"dsn",
	0
};
static struct net_address addr331[] = {
	012, 0270,
	044, 055,
	0, 0
};
static char *nic331[] = {
	"golden",
	"csl-arpanet-gateway",
	0
};
static struct net_address addr332[] = {
	026, 061641,
	0, 0
};
static char *nic332[] = {
	"gsb",
	0
};
static struct net_address addr333[] = {
	044, 050,
	0, 0
};
static char *nic333[] = {
	"helens",
	"vax3",
	0
};
static struct net_address addr334[] = {
	044, 050,
	0, 0
};
static char *nic334[] = {
	"diablo",
	"hnv",
	"vax2",
	0
};
static struct net_address addr335[] = {
	044, 050,
	0, 0
};
static char *nic335[] = {
	"lassen",
	"ifs",
	0
};
static struct net_address addr336[] = {
	044, 050,
	0, 0
};
static char *nic336[] = {
	"inyo",
	0
};
static struct net_address addr337[] = {
	044, 050,
	0, 0
};
static char *nic337[] = {
	"tamalpais",
	"isl",
	"isl-vax",
	"tam",
	0
};
static struct net_address addr338[] = {
	026, 037777712070,
	0, 0
};
static char *nic338[] = {
	"lots",
	0
};
static struct net_address addr339[] = {
	044, 050,
	0, 0
};
static char *nic339[] = {
	"marin",
	0
};
static struct net_address addr340[] = {
	044, 050,
	0, 0
};
static char *nic340[] = {
	"mojave",
	0
};
static struct net_address addr341[] = {
	044, 050,
	0, 0
};
static char *nic341[] = {
	"mono",
	0
};
static struct net_address addr342[] = {
	044, 050,
	0, 0
};
static char *nic342[] = {
	"monterey",
	0
};
static struct net_address addr343[] = {
	044, 050,
	0, 0
};
static char *nic343[] = {
	"napa",
	0
};
static struct net_address addr344[] = {
	044, 050,
	0, 0
};
static char *nic344[] = {
	"navajo",
	"na-vax",
	"na",
	0
};
static struct net_address addr345[] = {
	044, 050,
	0, 0
};
static char *nic345[] = {
	"pacifica",
	0
};
static struct net_address addr346[] = {
	044, 050,
	0, 0
};
static char *nic346[] = {
	"palomar",
	0
};
static struct net_address addr347[] = {
	044, 050,
	044, 055,
	0, 0
};
static char *nic347[] = {
	"sumex-mjh-gateway",
	"mjh-sumex-gateway",
	"pine",
	0
};
static struct net_address addr348[] = {
	044, 050,
	0, 0
};
static char *nic348[] = {
	"portola",
	0
};
static struct net_address addr349[] = {
	044, 050,
	0, 0
};
static char *nic349[] = {
	"tahoma",
	"su-tahoma",
	"psych-vax",
	"turtle-vax",
	0
};
static struct net_address addr350[] = {
	012, 0313,
	044, 050,
	0, 0
};
static char *nic350[] = {
	"score",
	0
};
static struct net_address addr351[] = {
	044, 050,
	044, 050,
	0, 0
};
static char *nic351[] = {
	"sequoia",
	0
};
static struct net_address addr352[] = {
	044, 050,
	0, 0
};
static char *nic352[] = {
	"shasta",
	"vax1",
	0
};
static struct net_address addr353[] = {
	044, 050,
	0, 0
};
static char *nic353[] = {
	"sun-24",
	0
};
static struct net_address addr354[] = {
	044, 050,
	0, 0
};
static char *nic354[] = {
	"sun-25",
	0
};
static struct net_address addr355[] = {
	044, 050,
	0, 0
};
static char *nic355[] = {
	"sun-26",
	0
};
static struct net_address addr356[] = {
	044, 050,
	0, 0
};
static char *nic356[] = {
	"sun-27",
	0
};
static struct net_address addr357[] = {
	044, 050,
	0, 0
};
static char *nic357[] = {
	"capitan",
	"sun-capitan",
	0
};
static struct net_address addr358[] = {
	044, 050,
	0, 0
};
static char *nic358[] = {
	"sun-cis",
	0
};
static struct net_address addr359[] = {
	044, 050,
	0, 0
};
static char *nic359[] = {
	"sun-donner",
	0
};
static struct net_address addr360[] = {
	044, 050,
	0, 0
};
static char *nic360[] = {
	"sun-ee",
	0
};
static struct net_address addr361[] = {
	044, 050,
	0, 0
};
static char *nic361[] = {
	"sun-erl257",
	0
};
static struct net_address addr362[] = {
	044, 050,
	0, 0
};
static char *nic362[] = {
	"sun-erl259",
	0
};
static struct net_address addr363[] = {
	044, 050,
	0, 0
};
static char *nic363[] = {
	"sun-erl406",
	0
};
static struct net_address addr364[] = {
	044, 050,
	0, 0
};
static char *nic364[] = {
	"sun-erl409",
	0
};
static struct net_address addr365[] = {
	044, 050,
	0, 0
};
static char *nic365[] = {
	"sun-gateway",
	0
};
static struct net_address addr366[] = {
	044, 050,
	0, 0
};
static char *nic366[] = {
	"halfdome",
	"sun-halfdome",
	0
};
static struct net_address addr367[] = {
	044, 050,
	0, 0
};
static char *nic367[] = {
	"sun-tip",
	"sun-mj020",
	0
};
static struct net_address addr368[] = {
	044, 050,
	0, 0
};
static char *nic368[] = {
	"sun-mj330",
	0
};
static struct net_address addr369[] = {
	044, 050,
	0, 0
};
static char *nic369[] = {
	"sun-mj342",
	0
};
static struct net_address addr370[] = {
	044, 050,
	0, 0
};
static char *nic370[] = {
	"sun-mj408",
	0
};
static struct net_address addr371[] = {
	044, 050,
	0, 0
};
static char *nic371[] = {
	"sun-mj416",
	0
};
static struct net_address addr372[] = {
	044, 050,
	0, 0
};
static char *nic372[] = {
	"sun-mh426",
	0
};
static struct net_address addr373[] = {
	044, 050,
	0, 0
};
static char *nic373[] = {
	"sun-mj428",
	0
};
static struct net_address addr374[] = {
	044, 050,
	0, 0
};
static char *nic374[] = {
	"sun-mj433",
	0
};
static struct net_address addr375[] = {
	044, 050,
	0, 0
};
static char *nic375[] = {
	"sun-mj460",
	0
};
static struct net_address addr376[] = {
	044, 055,
	0, 0
};
static char *nic376[] = {
	"sun-sumex-1",
	0
};
static struct net_address addr377[] = {
	044, 055,
	0, 0
};
static char *nic377[] = {
	"sun-sumex-2",
	0
};
static struct net_address addr378[] = {
	044, 055,
	0, 0
};
static char *nic378[] = {
	"sun-sumex-3",
	0
};
static struct net_address addr379[] = {
	044, 055,
	0, 0
};
static char *nic379[] = {
	"sun-sumex-tb113",
	0
};
static struct net_address addr380[] = {
	044, 050,
	0, 0
};
static char *nic380[] = {
	"tahoe",
	"dover",
	"su-dover",
	0
};
static struct net_address addr381[] = {
	044, 050,
	0, 0
};
static char *nic381[] = {
	"test-tip",
	0
};
static struct net_address addr382[] = {
	044, 050,
	0, 0
};
static char *nic382[] = {
	"tioga",
	"hnv-ethertip",
	0
};
static struct net_address addr383[] = {
	012, 0213,
	0, 0
};
static char *nic383[] = {
	"su-tip",
	0
};
static struct net_address addr384[] = {
	044, 050,
	0, 0
};
static char *nic384[] = {
	"toro",
	0
};
static struct net_address addr385[] = {
	044, 050,
	0, 0
};
static char *nic385[] = {
	"trinity",
	0
};
static struct net_address addr386[] = {
	044, 050,
	0, 0
};
static char *nic386[] = {
	"yolo",
	0
};
static struct net_address addr387[] = {
	044, 050,
	0, 0
};
static char *nic387[] = {
	"yosemite",
	0
};
static struct net_address addr388[] = {
	044, 055,
	0, 0
};
static char *nic388[] = {
	"aim-2020",
	"tiny",
	0
};
static struct net_address addr389[] = {
	012, 070,
	044, 055,
	0, 0
};
static char *nic389[] = {
	"aim",
	"sumex",
	"dual",
	0
};
static struct net_address addr390[] = {
	044, 055,
	044, 050,
	0, 0
};
static char *nic390[] = {
	"aim-alto",
	"palo",
	"madera",
	"su-madera",
	0
};
static struct net_address addr391[] = {
	044, 055,
	0, 0
};
static char *nic391[] = {
	"aim-dolphin-1",
	"a-dolph",
	0
};
static struct net_address addr392[] = {
	044, 050,
	0, 0
};
static char *nic392[] = {
	"aim-dolphin-2",
	"b-dolph",
	0
};
static struct net_address addr393[] = {
	044, 055,
	0, 0
};
static char *nic393[] = {
	"aim-dolphin-3",
	"c-dolph",
	0
};
static struct net_address addr394[] = {
	044, 050,
	0, 0
};
static char *nic394[] = {
	"aim-dolphin-4",
	"d-dolph",
	0
};
static struct net_address addr395[] = {
	044, 050,
	0, 0
};
static char *nic395[] = {
	"aim-dolphin-5",
	"e-dolph",
	0
};
static struct net_address addr396[] = {
	012, 0156,
	0, 0
};
static struct net_address addr397[] = {
	012, 071,
	0, 0
};
static char *nic397[] = {
	"nsa",
	0
};
static struct net_address addr398[] = {
	012, 0116,
	0, 0
};
static char *nic398[] = {
	"ucb-vax",
	0
};
static struct net_address addr399[] = {
	012, 0216,
	0, 0
};
static char *nic399[] = {
	"ucb",
	"berkeley",
	"berserkley",
	0
};
static struct net_address addr400[] = {
	012, 0242,
	0, 0
};
static char *nic400[] = {
	"ing70",
	0
};
static struct net_address addr401[] = {
	012, 0316,
	0, 0
};
static char *nic401[] = {
	"ingres",
	"ucb-ingres",
	0
};
static struct net_address addr402[] = {
	012, 01,
	0, 0
};
static char *nic402[] = {
	"ats",
	0
};
static struct net_address addr403[] = {
	012, 0101,
	0, 0
};
static char *nic403[] = {
	"ccn",
	0
};
static struct net_address addr404[] = {
	012, 0201,
	0, 0
};
static char *nic404[] = {
	"ucla",
	"ucla-s",
	"ucla-net",
	"insecurity",
	0
};
static struct net_address addr405[] = {
	012, 0140,
	0, 0
};
static char *nic405[] = {
	"ud-relay",
	"csnet-relay",
	"udel",
	"udee",
	"udel-ee",
	"delaware",
	"darcom-hq",
	0
};
static struct net_address addr406[] = {
	012, 0131,
	0, 0
};
static char *nic406[] = {
	"usafa",
	0
};
static struct net_address addr407[] = {
	012, 0327,
	0, 0
};
static char *nic407[] = {
	"ecl",
	"ecla",
	"usc-ecla",
	0
};
static struct net_address addr408[] = {
	012, 027,
	0, 0
};
static char *nic408[] = {
	"eclb",
	"usc-eclb-ipi",
	0
};
static struct net_address addr409[] = {
	012, 0131,
	0, 0
};
static char *nic409[] = {
	"eclc",
	0
};
static struct net_address addr410[] = {
	012, 0126,
	0, 0
};
static char *nic410[] = {
	"isia",
	"isi",
	"usc-isia",
	0
};
static struct net_address addr411[] = {
	012, 0364,
	0, 0
};
static char *nic411[] = {
	"isib",
	0
};
static struct net_address addr412[] = {
	012, 0226,
	0, 0
};
static char *nic412[] = {
	"isic",
	0
};
static struct net_address addr413[] = {
	012, 033,
	0, 0
};
static char *nic413[] = {
	"isid",
	0
};
static struct net_address addr414[] = {
	012, 0164,
	0, 0
};
static char *nic414[] = {
	"isie",
	0
};
static struct net_address addr415[] = {
	012, 0264,
	0, 0
};
static char *nic415[] = {
	"isif",
	0
};
static struct net_address addr416[] = {
	012, 0227,
	0, 0
};
static struct net_address addr417[] = {
	012, 0304,
	0, 0
};
static char *nic417[] = {
	"reston-amdahl",
	"ram",
	0
};
static struct net_address addr418[] = {
	012, 0104,
	0, 0
};
static char *nic418[] = {
	"reston",
	"rest",
	0
};
static struct net_address addr419[] = {
	012, 0404,
	0, 0
};
static struct net_address addr420[] = {
	012, 0105,
	0, 0
};
static char *nic420[] = {
	"denver",
	0
};
static struct net_address addr421[] = {
	012, 0205,
	0, 0
};
static struct net_address addr422[] = {
	012, 0106,
	0, 0
};
static struct net_address addr423[] = {
	012, 0206,
	0, 0
};
static struct net_address addr424[] = {
	012, 0304,
	0, 0
};
static char *nic424[] = {
	"utah",
	0
};
static struct net_address addr425[] = {
	012, 0204,
	0, 0
};
static struct net_address addr426[] = {
	012, 076,
	0, 0
};
static char *nic426[] = {
	"utex",
	0
};
static struct net_address addr427[] = {
	012, 0176,
	0, 0
};
static char *nic427[] = {
	"utexas",
	"texas-20",
	0
};
static struct net_address addr428[] = {
	012, 0136,
	0, 0
};
static char *nic428[] = {
	"wisconsin",
	0
};
static struct net_address addr429[] = {
	012, 0133,
	0, 0
};
static char *nic429[] = {
	"uwash",
	"uw",
	"udub-ward",
	"udub",
	0
};
static struct net_address addr430[] = {
	012, 0333,
	0, 0
};
static struct net_address addr431[] = {
	012, 0330,
	0, 0
};
static char *nic431[] = {
	"wharton",
	0
};
static struct net_address addr432[] = {
	012, 057,
	0, 0
};
static struct net_address addr433[] = {
	012, 0157,
	0, 0
};
static char *nic433[] = {
	"avsail",
	"wpafb-afal",
	0
};
static struct net_address addr434[] = {
	012, 0257,
	0, 0
};
static struct net_address addr435[] = {
	012, 0300,
	0, 0
};
static struct net_address addr436[] = {
	012, 0312,
	0, 0
};
static struct net_address addr437[] = {
	012, 0211,
	0, 0
};
static struct net_address addr438[] = {
	012, 0113,
	0, 0
};
struct host_entry host_table[] = {
	{"acc",	addr0,	0,	h_s0,	h_m0,	0},
	{"accat-tip",	addr1,	0,	h_s1,	h_m1,	nic1},
	{"ada-vax",	addr2,	1,	h_s2,	h_m2,	nic2},
	{"aerospace",	addr3,	1,	h_s0,	h_m2,	nic3},
	{"afgl",	addr4,	1,	h_s3,	h_m3,	0},
	{"afgl-tac",	addr5,	0,	h_s4,	h_m1,	0},
	{"afsc-ad",	addr6,	1,	h_s3,	h_m3,	nic6},
	{"afsc-dev",	addr7,	1,	h_s5,	h_m0,	nic7},
	{"afsc-hq",	addr8,	1,	h_s6,	h_m4,	nic8},
	{"afsc-hq-tac",	addr9,	0,	h_s4,	h_m1,	nic9},
	{"afsc-sd",	addr10,	1,	h_s6,	h_m4,	nic10},
	{"afsc-sd-tac",	addr11,	0,	h_s4,	h_m1,	nic11},
	{"afwl",	addr12,	1,	h_s7,	h_m3,	nic12},
	{"afwl-tip",	addr13,	0,	h_s1,	h_m1,	nic13},
	{"ames-11",	addr14,	1,	h_s2,	h_m2,	0},
	{"ames-67",	addr15,	1,	h_s8,	h_m5,	nic15},
	{"ames-tip",	addr16,	0,	h_s1,	h_m1,	0},
	{"anl",	addr17,	1,	h_s9,	h_m6,	nic17},
	{"arpa-dms",	addr18,	1,	h_s10,	h_m7,	0},
	{"arpa-penguin",	addr19,	1,	h_s11,	h_m0,	nic19},
	{"arpa-tip",	addr20,	0,	h_s1,	h_m1,	0},
	{"bbn-admin",	addr21,	1,	h_s0,	h_m8,	nic21},
	{"bbn-clxx",	addr22,	1,	h_s0,	h_m8,	nic22},
	{"bbn-gateway",	addr23,	0,	h_s11,	h_m0,	0},
	{"bbn-ig",	addr24,	0,	h_s12,	h_m9,	0},
	{"bbn-itnc",	addr25,	1,	h_s6,	h_m4,	nic25},
	{"bbn-ncc",	addr26,	0,	h_s13,	h_m1,	nic26},
	{"bbn-nu",	addr27,	1,	h_s0,	h_m8,	nic27},
	{"bbn-ptip",	addr28,	0,	h_s1,	h_m10,	nic28},
	{"bbn-rsm",	addr29,	1,	h_s0,	h_m0,	nic29},
	{"bbn-speech-11",	addr30,	0,	h_s11,	h_m0,	0},
	{"bbn-tac",	addr31,	0,	h_s4,	h_m1,	0},
	{"bbn-testip",	addr32,	0,	h_s14,	h_m1,	0},
	{"bbn-unix",	addr33,	1,	h_s0,	h_m8,	nic33},
	{"bbn-vax",	addr34,	1,	h_s0,	h_m2,	nic34},
	{"bbna",	addr35,	1,	h_s6,	h_m4,	nic35},
	{"bbnb",	addr36,	1,	h_s15,	h_m4,	nic36},
	{"bbnc",	addr37,	1,	h_s15,	h_m4,	nic37},
	{"bbncca",	addr38,	1,	h_s0,	h_m8,	nic38},
	{"bbnccb",	addr39,	1,	h_s0,	h_m8,	0},
	{"bbnf",	addr40,	1,	h_s6,	h_m4,	nic40},
	{"bbng",	addr41,	1,	h_s6,	h_m4,	nic41},
	{"bbnp",	addr42,	1,	h_s0,	h_m8,	0},
	{"bbnq",	addr43,	1,	h_s0,	h_m8,	0},
	{"bbns",	addr44,	1,	h_s0,	h_m8,	0},
	{"bbnt",	addr45,	1,	h_s0,	h_m8,	0},
	{"bbnw",	addr46,	1,	h_s0,	h_m8,	0},
	{"bnl",	addr47,	1,	h_s7,	h_m11,	nic47},
	{"bragg-gwy1",	addr48,	0,	h_s0,	h_m0,	0},
	{"bragg-sta1",	addr49,	1,	h_s12,	h_m9,	0},
	{"bragg-tac",	addr50,	0,	h_s4,	h_m1,	0},
	{"brl",	addr51,	1,	h_s0,	h_m0,	0},
	{"brl-bmd",	addr52,	1,	h_s0,	h_m0,	nic52},
	{"brl-tip",	addr53,	0,	h_s1,	h_m1,	0},
	{"cca-tac",	addr54,	0,	h_s4,	h_m1,	nic54},
	{"cca-unix",	addr55,	1,	h_s0,	h_m2,	nic55},
	{"cca-vms",	addr56,	1,	h_s2,	h_m2,	0},
	{"cctc",	addr57,	1,	h_s0,	h_m0,	0},
	{"centacs-mmp",	addr58,	0,	h_s16,	h_m12,	0},
	{"centacs-tf",	addr59,	0,	h_s17,	h_m0,	0},
	{"chi",	addr60,	0,	h_s18,	h_m13,	nic60},
	{"cincpac-tip",	addr61,	0,	h_s1,	h_m1,	nic61},
	{"cincpacflt-wm",	addr62,	0,	h_s0,	h_m0,	0},
	{"cit-20",	addr63,	1,	h_s6,	h_m4,	nic63},
	{"cit-vax",	addr64,	0,	h_s0,	h_m0,	nic64},
	{"clarksburg-ig",	addr65,	0,	h_s12,	h_m9,	0},
	{"cmu-10a",	addr66,	1,	h_s19,	h_m4,	nic66},
	{"cmu-10b",	addr67,	1,	h_s19,	h_m4,	nic67},
	{"cmu-20c",	addr68,	1,	h_s6,	h_m4,	nic68},
	{"cmu-gateway",	addr69,	0,	h_s12,	h_m9,	nic69},
	{"coins-gateway",	addr70,	0,	h_s0,	h_m0,	nic70},
	{"coins-tas",	addr71,	0,	h_s0,	h_m0,	0},
	{"collins-pr",	addr72,	0,	h_s20,	h_m0,	0},
	{"collins-tip",	addr73,	0,	h_s1,	h_m1,	0},
	{"comsat-mtr",	addr74,	0,	h_s12,	h_m9,	0},
	{"coradcom-tip",	addr75,	0,	h_s1,	h_m1,	0},
	{"csnet-purdue",	addr76,	1,	h_s0,	h_m2,	nic76},
	{"csnet-sh",	addr77,	1,	h_s0,	h_m2,	nic77},
	{"darcom-ka",	addr78,	1,	h_s15,	h_m4,	nic78},
	{"darcom-tac",	addr79,	0,	h_s4,	h_m1,	0},
	{"david-tac",	addr80,	0,	h_s4,	h_m1,	nic80},
	{"dcec-tip",	addr81,	0,	h_s1,	h_m1,	0},
	{"dec-2136",	addr82,	1,	h_s6,	h_m4,	nic82},
	{"dec-marlboro",	addr83,	1,	h_s6,	h_m4,	nic83},
	{"dti",	addr84,	1,	h_s0,	h_m0,	0},
	{"dti-vms",	addr85,	1,	h_s2,	h_m2,	0},
	{"dtnsrdc",	addr86,	1,	h_s3,	h_m3,	nic86},
	{"edn-unix",	addr87,	1,	h_s0,	h_m0,	0},
	{"etac",	addr88,	0,	h_s11,	h_m0,	0},
	{"fnoc",	addr89,	0,	h_s7,	h_m14,	nic89},
	{"fnoc-secure",	addr90,	0,	h_s7,	h_m14,	nic90},
	{"gunter-adam",	addr91,	1,	h_s6,	h_m4,	nic91},
	{"gunter-tac",	addr92,	0,	h_s4,	h_m1,	nic92},
	{"gunter-unix",	addr93,	1,	h_s0,	h_m0,	nic93},
	{"harv-10",	addr94,	1,	h_s19,	h_m4,	nic94},
	{"hewlett-packard",	addr95,	1,	h_s6,	h_m4,	nic95},
	{"hi-multics",	addr96,	1,	h_s21,	h_m15,	nic96},
	{"isi-mincon",	addr97,	0,	h_s11,	h_m9,	0},
	{"isi-png11",	addr98,	0,	h_s11,	h_m0,	nic98},
	{"isi-psat",	addr99,	0,	h_s12,	h_m9,	0},
	{"isi-speech11",	addr100,	0,	h_s0,	h_m0,	0},
	{"isi-vaxa",	addr101,	1,	h_s0,	h_m2,	nic101},
	{"isi-wbc11",	addr102,	0,	h_s11,	h_m0,	0},
	{"jpl-vax",	addr103,	1,	h_s12,	h_m2,	0},
	{"kestrel",	addr104,	1,	h_s15,	h_m4,	nic104},
	{"lbl",	addr105,	1,	h_s22,	h_m11,	0},
	{"lbl-unix",	addr106,	1,	h_s0,	h_m2,	0},
	{"ll",	addr107,	1,	h_s23,	h_m16,	0},
	{"ll-11",	addr108,	1,	h_s0,	h_m0,	0},
	{"ll-asg",	addr109,	1,	h_s0,	h_m0,	0},
	{"ll-psat",	addr110,	0,	h_s12,	h_m9,	0},
	{"ll-xn",	addr111,	1,	h_s0,	h_m0,	0},
	{"lll-mfe",	addr112,	1,	h_s19,	h_m4,	nic112},
	{"lll-unix",	addr113,	1,	h_s0,	h_m0,	nic113},
	{"logicon",	addr114,	1,	h_s0,	h_m0,	0},
	{"london",	addr115,	1,	h_s9,	h_m17,	nic115},
	{"london-gateway",	addr116,	0,	h_s11,	h_m0,	nic116},
	{"london-tac",	addr117,	0,	h_s4,	h_m1,	0},
	{"martin",	addr118,	1,	h_s5,	h_m0,	0},
	{"mcclellan",	addr119,	0,	h_s12,	h_m9,	0},
	{"mit-ai",	addr120,	1,	h_s24,	h_m4,	nic120},
	{"mit-ai-11",	addr121,	0,	h_s25,	h_m0,	nic121},
	{"mit-ajax",	addr122,	1,	h_s0,	h_m2,	nic122},
	{"mit-alcator",	addr123,	1,	h_s2,	h_m2,	nic123},
	{"mit-arcmac",	addr124,	1,	h_s26,	h_m18,	nic124},
	{"mit-arcmac-nick",	addr125,	1,	h_s26,	h_m18,	nic125},
	{"mit-arthur",	addr126,	0,	h_s27,	h_m19,	nic126},
	{"mit-benji-mouse",	addr127,	0,	h_s28,	h_m0,	nic127},
	{"mit-bridge",	addr128,	0,	h_s17,	h_m0,	nic128},
	{"mit-bypass",	addr129,	0,	h_s28,	h_m0,	nic129},
	{"mit-ccc",	addr130,	1,	h_s0,	h_m0,	nic130},
	{"mit-cipg",	addr131,	1,	h_s0,	h_m0,	nic131},
	{"mit-cog-sci",	addr132,	1,	h_s0,	h_m0,	nic132},
	{"mit-csg",	addr133,	1,	h_s0,	h_m0,	nic133},
	{"mit-csr",	addr134,	1,	h_s0,	h_m0,	nic134},
	{"mit-devmultics",	addr135,	1,	h_s21,	h_m20,	nic135},
	{"mit-dms",	addr136,	1,	h_s24,	h_m4,	nic136},
	{"mit-dspg",	addr137,	1,	h_s0,	h_m0,	nic137},
	{"mit-eddie",	addr138,	1,	h_s2,	h_m2,	nic138},
	{"mit-eecs",	addr139,	1,	h_s6,	h_m4,	nic139},
	{"mit-eecs-11",	addr140,	0,	h_s25,	h_m0,	nic140},
	{"mit-eetest",	addr141,	0,	h_s28,	h_m0,	nic141},
	{"mit-ford",	addr142,	0,	h_s27,	h_m19,	nic142},
	{"mit-franky-mouse",	addr143,	0,	h_s28,	h_m0,	nic143},
	{"mit-fusion",	addr144,	0,	h_s29,	h_m0,	nic144},
	{"mit-gw",	addr145,	0,	h_s17,	h_m0,	nic145},
	{"mit-htjr",	addr146,	1,	h_s2,	h_m2,	nic146},
	{"mit-htvax",	addr147,	1,	h_s0,	h_m2,	nic147},
	{"mit-jcf-11",	addr148,	0,	h_s30,	h_m0,	nic148},
	{"mit-lispm-1",	addr149,	0,	h_s27,	h_m19,	nic149},
	{"mit-lispm-2",	addr150,	0,	h_s27,	h_m19,	nic150},
	{"mit-lispm-3",	addr151,	0,	h_s27,	h_m19,	nic151},
	{"mit-lispm-4",	addr152,	0,	h_s27,	h_m19,	nic152},
	{"mit-lispm-5",	addr153,	0,	h_s27,	h_m19,	nic153},
	{"mit-lispm-6",	addr154,	0,	h_s27,	h_m19,	nic154},
	{"mit-lispm-7",	addr155,	0,	h_s27,	h_m19,	nic155},
	{"mit-lispm-8",	addr156,	0,	h_s27,	h_m19,	nic156},
	{"mit-lispm-9",	addr157,	0,	h_s27,	h_m19,	nic157},
	{"mit-lispm-12",	addr158,	0,	h_s27,	h_m19,	nic158},
	{"mit-lispm-15",	addr159,	0,	h_s27,	h_m19,	nic159},
	{"mit-lispm-16",	addr160,	0,	h_s27,	h_m19,	nic160},
	{"mit-lispm-18",	addr161,	0,	h_s27,	h_m19,	nic161},
	{"mit-lispm-19",	addr162,	0,	h_s27,	h_m19,	nic162},
	{"mit-lispm-20",	addr163,	0,	h_s27,	h_m19,	nic163},
	{"mit-lispm-22",	addr164,	0,	h_s27,	h_m19,	nic164},
	{"mit-lispm-23",	addr165,	0,	h_s27,	h_m19,	nic165},
	{"mit-lispm-24",	addr166,	0,	h_s27,	h_m19,	nic166},
	{"mit-lispm-25",	addr167,	0,	h_s27,	h_m19,	nic167},
	{"mit-lispm-26",	addr168,	0,	h_s27,	h_m19,	nic168},
	{"mit-lispm-27",	addr169,	1,	h_s27,	h_m19,	nic169},
	{"mit-lispm-29",	addr170,	0,	h_s27,	h_m19,	nic170},
	{"mit-lispm-test",	addr171,	0,	h_s27,	h_m19,	nic171},
	{"mit-lns-vax",	addr172,	1,	h_s2,	h_m2,	nic172},
	{"mit-lns-vax-2",	addr173,	1,	h_s2,	h_m2,	nic173},
	{"mit-marvin",	addr174,	0,	h_s27,	h_m19,	nic174},
	{"mit-math",	addr175,	1,	h_s0,	h_m0,	nic175},
	{"mit-mc",	addr176,	1,	h_s24,	h_m4,	nic176},
	{"mit-mc-11",	addr177,	0,	h_s25,	h_m0,	nic177},
	{"mit-ml",	addr178,	1,	h_s24,	h_m4,	nic178},
	{"mit-mr11",	addr179,	0,	h_s31,	h_m0,	0},
	{"mit-multics",	addr180,	1,	h_s21,	h_m21,	nic180},
	{"mit-multics-11",	addr181,	1,	h_s21,	h_m0,	nic181},
	{"mit-nplasma",	addr182,	0,	h_s29,	h_m0,	nic182},
	{"mit-oz",	addr183,	1,	h_s6,	h_m4,	nic183},
	{"mit-oz-11",	addr184,	0,	h_s28,	h_m0,	nic184},
	{"mit-oz-tip-1",	addr185,	0,	h_s28,	h_m0,	nic185},
	{"mit-oz-tip-2",	addr186,	0,	h_s28,	h_m0,	nic186},
	{"mit-pfc-test",	addr187,	0,	h_s28,	h_m0,	nic187},
	{"mit-pfc-vax",	addr188,	1,	h_s2,	h_m2,	nic188},
	{"mit-pfc-versatec",	addr189,	0,	h_s28,	h_m0,	nic189},
	{"mit-plasma",	addr190,	0,	h_s28,	h_m0,	nic190},
	{"mit-rts",	addr191,	1,	h_s0,	h_m0,	nic191},
	{"mit-rts-tip-1",	addr192,	0,	h_s28,	h_m0,	nic192},
	{"mit-rts-tip-2",	addr193,	0,	h_s28,	h_m0,	nic193},
	{"mit-rts-tip-3",	addr194,	0,	h_s28,	h_m0,	nic194},
	{"mit-rts-tip-4",	addr195,	0,	h_s28,	h_m0,	nic195},
	{"mit-rts-tip-5",	addr196,	0,	h_s28,	h_m0,	nic196},
	{"mit-scrc-microwave",	addr197,	0,	h_s28,	h_m0,	nic197},
	{"mit-sipb",	addr198,	0,	h_s28,	h_m0,	nic198},
	{"mit-speech",	addr199,	1,	h_s6,	h_m4,	nic199},
	{"mit-speech-11",	addr200,	0,	h_s28,	h_m0,	nic200},
	{"mit-spooler",	addr201,	1,	h_s32,	h_m22,	nic201},
	{"mit-student-center",	addr202,	0,	h_s28,	h_m0,	nic202},
	{"mit-tip",	addr203,	0,	h_s1,	h_m1,	0},
	{"mit-tiu",	addr204,	0,	h_s17,	h_m0,	nic204},
	{"mit-tst",	addr205,	0,	h_s17,	h_m0,	nic205},
	{"mit-vax",	addr206,	1,	h_s0,	h_m2,	nic206},
	{"mit-xx",	addr207,	1,	h_s6,	h_m4,	nic207},
	{"mit-xx-11",	addr208,	0,	h_s25,	h_m0,	nic208},
	{"mit-zaphod",	addr209,	0,	h_s27,	h_m19,	nic209},
	{"mit-zarniwoop",	addr210,	0,	h_s27,	h_m19,	nic210},
	{"mitre",	addr211,	1,	h_s0,	h_m0,	0},
	{"mitre-bedford",	addr212,	1,	h_s0,	h_m0,	0},
	{"mitre-tip",	addr213,	0,	h_s1,	h_m1,	0},
	{"moffett-arc-a",	addr214,	1,	h_s15,	h_m4,	nic214},
	{"moffett-arc-b",	addr215,	0,	h_s33,	h_m10,	nic215},
	{"nadc",	addr216,	1,	h_s34,	h_m14,	0},
	{"nalcon",	addr217,	1,	h_s12,	h_m9,	nic217},
	{"nbs-pl",	addr218,	1,	h_s0,	h_m0,	nic218},
	{"nbs-sdc",	addr219,	1,	h_s2,	h_m2,	0},
	{"nbs-tip",	addr220,	0,	h_s1,	h_m1,	0},
	{"nbs-vms",	addr221,	1,	h_s2,	h_m2,	nic221},
	{"ncc-tip",	addr222,	0,	h_s1,	h_m1,	0},
	{"ncsc",	addr223,	1,	h_s0,	h_m0,	nic223},
	{"nlm-mcs",	addr224,	1,	h_s6,	h_m4,	nic224},
	{"nosc-cc",	addr225,	1,	h_s35,	h_m23,	nic225},
	{"nosc-sdl",	addr226,	1,	h_s0,	h_m0,	nic226},
	{"nosc-secure2",	addr227,	0,	h_s15,	h_m4,	nic227},
	{"nosc-secure3",	addr228,	0,	h_s0,	h_m0,	0},
	{"nosc-spel",	addr229,	1,	h_s0,	h_m0,	nic229},
	{"nprdc",	addr230,	1,	h_s0,	h_m0,	nic230},
	{"nps",	addr231,	1,	h_s0,	h_m0,	0},
	{"nps-tip",	addr232,	0,	h_s1,	h_m1,	0},
	{"nrl",	addr233,	1,	h_s0,	h_m0,	0},
	{"nrl-arctan",	addr234,	1,	h_s36,	h_m0,	0},
	{"nrl-css",	addr235,	1,	h_s0,	h_m0,	nic235},
	{"nrl-tops10",	addr236,	1,	h_s19,	h_m4,	0},
	{"nswc-dl",	addr237,	1,	h_s37,	h_m0,	0},
	{"nswc-tac",	addr238,	0,	h_s4,	h_m1,	0},
	{"nswc-wo",	addr239,	1,	h_s38,	h_m14,	0},
	{"nusc",	addr240,	1,	h_s2,	h_m2,	0},
	{"nusc-npt",	addr241,	1,	h_s0,	h_m0,	nic241},
	{"nwc-387a",	addr242,	1,	h_s12,	h_m9,	0},
	{"nwc-387b",	addr243,	1,	h_s12,	h_m9,	0},
	{"nwc-elf",	addr244,	1,	h_s11,	h_m0,	nic244},
	{"nwc-tac",	addr245,	0,	h_s4,	h_m1,	0},
	{"nyu",	addr246,	1,	h_s2,	h_m2,	0},
	{"office-1",	addr247,	1,	h_s15,	h_m4,	nic247},
	{"office-2",	addr248,	1,	h_s15,	h_m4,	nic248},
	{"office-3",	addr249,	1,	h_s15,	h_m4,	nic249},
	{"office-7",	addr250,	1,	h_s15,	h_m4,	nic250},
	{"office-8",	addr251,	1,	h_s15,	h_m4,	nic251},
	{"okc-unix",	addr252,	1,	h_s0,	h_m9,	0},
	{"parc-maxc",	addr253,	1,	h_s15,	h_m4,	nic253},
	{"pent-unix",	addr254,	0,	h_s0,	h_m0,	0},
	{"pentagon-tip",	addr255,	0,	h_s1,	h_m1,	0},
	{"pred-unix",	addr256,	1,	h_s0,	h_m8,	0},
	{"purdue",	addr257,	1,	h_s0,	h_m2,	nic257},
	{"radc-multics",	addr258,	1,	h_s21,	h_m15,	nic258},
	{"radc-tip",	addr259,	0,	h_s1,	h_m1,	nic259},
	{"radc-tops20",	addr260,	1,	h_s6,	h_m4,	nic260},
	{"radc-unix",	addr261,	1,	h_s0,	h_m0,	0},
	{"radc-xper",	addr262,	1,	h_s0,	h_m0,	nic262},
	{"rand-ai",	addr263,	1,	h_s6,	h_m4,	nic263},
	{"rand-relay",	addr264,	1,	h_s0,	h_m2,	0},
	{"rand-tip",	addr265,	0,	h_s1,	h_m1,	0},
	{"rand-unix",	addr266,	1,	h_s0,	h_m2,	nic266},
	{"robins-unix",	addr267,	1,	h_s0,	h_m0,	0},
	{"rochester",	addr268,	0,	h_s39,	h_m24,	nic268},
	{"ru-green",	addr269,	1,	h_s6,	h_m4,	nic269},
	{"rutgers",	addr270,	1,	h_s6,	h_m4,	nic270},
	{"s1-a",	addr271,	1,	h_s40,	h_m4,	nic271},
	{"s1-b",	addr272,	0,	h_s12,	h_m9,	0},
	{"s1-c",	addr273,	0,	h_s12,	h_m9,	0},
	{"s1-gateway",	addr274,	0,	h_s12,	h_m0,	0},
	{"s1-gswit",	addr275,	0,	h_s41,	h_m0,	0},
	{"s1-lswit",	addr276,	0,	h_s41,	h_m0,	0},
	{"s1-nswit",	addr277,	0,	h_s41,	h_m0,	0},
	{"sac-tac",	addr278,	0,	h_s4,	h_m1,	0},
	{"sandia",	addr279,	1,	h_s6,	h_m4,	nic279},
	{"sch-donner",	addr280,	0,	h_s28,	h_m0,	nic280},
	{"scrc-afghan",	addr281,	0,	h_s27,	h_m19,	nic281},
	{"scrc-basset",	addr282,	0,	h_s27,	h_m19,	nic282},
	{"scrc-beagle",	addr283,	0,	h_s27,	h_m19,	nic283},
	{"scrc-borzoi",	addr284,	0,	h_s27,	h_m19,	nic284},
	{"scrc-boxer",	addr285,	0,	h_s27,	h_m19,	nic285},
	{"scrc-bulldog",	addr286,	0,	h_s27,	h_m19,	nic286},
	{"scrc-collie",	addr287,	0,	h_s27,	h_m19,	nic287},
	{"scrc-comet",	addr288,	1,	h_s2,	h_m2,	nic288},
	{"scrc-dachshund",	addr289,	0,	h_s27,	h_m19,	nic289},
	{"scrc-dalmatian",	addr290,	0,	h_s27,	h_m19,	nic290},
	{"scrc-dancer",	addr291,	0,	h_s28,	h_m0,	nic291},
	{"scrc-husky",	addr292,	0,	h_s27,	h_m19,	nic292},
	{"scrc-pointer",	addr293,	0,	h_s27,	h_m19,	nic293},
	{"scrc-prancer",	addr294,	0,	h_s28,	h_m0,	nic294},
	{"scrc-retriever",	addr295,	0,	h_s27,	h_m19,	nic295},
	{"scrc-samoyed",	addr296,	0,	h_s27,	h_m19,	nic296},
	{"scrc-schnauzer",	addr297,	0,	h_s27,	h_m19,	nic297},
	{"scrc-setter",	addr298,	0,	h_s27,	h_m19,	nic298},
	{"scrc-shepherd",	addr299,	0,	h_s27,	h_m19,	nic299},
	{"scrc-spaniel",	addr300,	0,	h_s27,	h_m19,	nic300},
	{"scrc-tenex",	addr301,	1,	h_s15,	h_m4,	nic301},
	{"scrc-terrier",	addr302,	0,	h_s27,	h_m19,	nic302},
	{"scrc-tv-11",	addr303,	0,	h_s29,	h_m0,	nic303},
	{"scrc-vixen",	addr304,	1,	h_s0,	h_m2,	nic304},
	{"scrc-zippy",	addr305,	0,	h_s27,	h_m19,	nic305},
	{"spa-blitzen",	addr306,	0,	h_s28,	h_m0,	nic306},
	{"src-44",	addr307,	1,	h_s42,	h_m25,	nic307},
	{"src-ccp",	addr308,	0,	h_s12,	h_m10,	nic308},
	{"src-nep",	addr309,	0,	h_s42,	h_m26,	nic309},
	{"src-unix",	addr310,	1,	h_s0,	h_m0,	nic310},
	{"sri-ai",	addr311,	1,	h_s6,	h_m4,	nic311},
	{"sri-c3po",	addr312,	1,	h_s15,	h_m4,	nic312},
	{"sri-csl",	addr313,	1,	h_s15,	h_m4,	nic313},
	{"sri-iu",	addr314,	1,	h_s2,	h_m2,	nic314},
	{"sri-kl",	addr315,	1,	h_s6,	h_m4,	nic315},
	{"sri-nic",	addr316,	1,	h_s15,	h_m4,	nic316},
	{"sri-nsc11",	addr317,	1,	h_s0,	h_m0,	nic317},
	{"sri-r2d2",	addr318,	1,	h_s0,	h_m0,	nic318},
	{"sri-tsc",	addr319,	1,	h_s0,	h_m0,	nic319},
	{"sri-unix",	addr320,	1,	h_s0,	h_m0,	nic320},
	{"sri-warf",	addr321,	1,	h_s0,	h_m0,	nic321},
	{"stla-tac",	addr322,	0,	h_s4,	h_m1,	nic322},
	{"su-1145",	addr323,	0,	h_s43,	h_m0,	nic323},
	{"su-ai",	addr324,	1,	h_s40,	h_m4,	nic324},
	{"su-almanor",	addr325,	0,	h_s44,	h_m22,	nic325},
	{"su-cit1",	addr326,	0,	h_s45,	h_m27,	nic326},
	{"su-cit2",	addr327,	0,	h_s45,	h_m27,	nic327},
	{"su-cypress",	addr328,	0,	h_s44,	h_m22,	nic328},
	{"su-diego",	addr329,	0,	h_s44,	h_m22,	nic329},
	{"su-dsn",	addr330,	1,	h_s0,	h_m2,	nic330},
	{"su-golden",	addr331,	0,	h_s17,	h_m0,	nic331},
	{"su-gsb",	addr332,	1,	h_s6,	h_m4,	nic332},
	{"su-helens",	addr333,	1,	h_s0,	h_m2,	nic333},
	{"su-hnv",	addr334,	1,	h_s0,	h_m2,	nic334},
	{"su-ifs",	addr335,	1,	h_s46,	h_m22,	nic335},
	{"su-inyo",	addr336,	0,	h_s44,	h_m22,	nic336},
	{"su-isl",	addr337,	1,	h_s0,	h_m2,	nic337},
	{"su-lots",	addr338,	1,	h_s6,	h_m4,	nic338},
	{"su-marin",	addr339,	0,	h_s44,	h_m22,	nic339},
	{"su-mojave",	addr340,	0,	h_s44,	h_m22,	nic340},
	{"su-mono",	addr341,	0,	h_s44,	h_m22,	nic341},
	{"su-monterey",	addr342,	0,	h_s44,	h_m22,	nic342},
	{"su-napa",	addr343,	0,	h_s44,	h_m22,	nic343},
	{"su-navajo",	addr344,	1,	h_s0,	h_m2,	nic344},
	{"su-pacifica",	addr345,	0,	h_s47,	h_m28,	nic345},
	{"su-palomar",	addr346,	0,	h_s44,	h_m22,	nic346},
	{"su-pine",	addr347,	0,	h_s48,	h_m0,	nic347},
	{"su-portola",	addr348,	0,	h_s44,	h_m22,	nic348},
	{"su-psych",	addr349,	1,	h_s0,	h_m2,	nic349},
	{"su-score",	addr350,	1,	h_s6,	h_m4,	nic350},
	{"su-sequoia",	addr351,	0,	h_s49,	h_m29,	nic351},
	{"su-shasta",	addr352,	1,	h_s0,	h_m2,	nic352},
	{"su-sun-24",	addr353,	0,	h_s49,	h_m30,	nic353},
	{"su-sun-25",	addr354,	0,	h_s49,	h_m30,	nic354},
	{"su-sun-26",	addr355,	0,	h_s49,	h_m30,	nic355},
	{"su-sun-27",	addr356,	0,	h_s49,	h_m30,	nic356},
	{"su-sun-capitan",	addr357,	0,	h_s49,	h_m30,	nic357},
	{"su-sun-cis",	addr358,	0,	h_s50,	h_m30,	nic358},
	{"su-sun-donner",	addr359,	0,	h_s49,	h_m30,	nic359},
	{"su-sun-ee",	addr360,	0,	h_s50,	h_m30,	nic360},
	{"su-sun-erl257",	addr361,	0,	h_s49,	h_m30,	nic361},
	{"su-sun-erl259",	addr362,	0,	h_s49,	h_m30,	nic362},
	{"su-sun-erl406",	addr363,	0,	h_s49,	h_m30,	nic363},
	{"su-sun-erl409",	addr364,	0,	h_s49,	h_m30,	nic364},
	{"su-sun-gateway",	addr365,	0,	h_s49,	h_m30,	nic365},
	{"su-sun-halfdome",	addr366,	0,	h_s49,	h_m30,	nic366},
	{"su-sun-mj020",	addr367,	0,	h_s50,	h_m30,	nic367},
	{"su-sun-mj330",	addr368,	0,	h_s49,	h_m30,	nic368},
	{"su-sun-mj342",	addr369,	0,	h_s49,	h_m30,	nic369},
	{"su-sun-mj408",	addr370,	0,	h_s49,	h_m30,	nic370},
	{"su-sun-mj416",	addr371,	0,	h_s49,	h_m30,	nic371},
	{"su-sun-mj426",	addr372,	0,	h_s50,	h_m30,	nic372},
	{"su-sun-mj428",	addr373,	0,	h_s49,	h_m30,	nic373},
	{"su-sun-mj433",	addr374,	0,	h_s49,	h_m30,	nic374},
	{"su-sun-mj460",	addr375,	0,	h_s49,	h_m30,	nic375},
	{"su-sun-sumex-1",	addr376,	0,	h_s49,	h_m30,	nic376},
	{"su-sun-sumex-2",	addr377,	0,	h_s49,	h_m30,	nic377},
	{"su-sun-sumex-3",	addr378,	0,	h_s49,	h_m30,	nic378},
	{"su-sun-sumex-tb113",	addr379,	0,	h_s49,	h_m30,	nic379},
	{"su-tahoe",	addr380,	0,	h_s51,	h_m22,	nic380},
	{"su-test-tip",	addr381,	0,	h_s50,	h_m30,	nic381},
	{"su-tioga",	addr382,	0,	h_s52,	h_m0,	nic382},
	{"su-tac",	addr383,	0,	h_s4,	h_m1,	nic383},
	{"su-toro",	addr384,	0,	h_s44,	h_m22,	nic384},
	{"su-trinity",	addr385,	0,	h_s44,	h_m22,	nic385},
	{"su-yolo",	addr386,	0,	h_s44,	h_m22,	nic386},
	{"su-yosemite",	addr387,	0,	h_s44,	h_m22,	nic387},
	{"sumex-2020",	addr388,	1,	h_s6,	h_m4,	nic388},
	{"sumex-aim",	addr389,	1,	h_s15,	h_m4,	nic389},
	{"sumex-alto",	addr390,	0,	h_s44,	h_m22,	nic390},
	{"sumex-dolphin-1",	addr391,	0,	h_s47,	h_m28,	nic391},
	{"sumex-dolphin-2",	addr392,	0,	h_s47,	h_m28,	nic392},
	{"sumex-dolphin-3",	addr393,	0,	h_s47,	h_m28,	nic393},
	{"sumex-dolphin-4",	addr394,	0,	h_s47,	h_m28,	nic394},
	{"sumex-dolphin-5",	addr395,	0,	h_s47,	h_m28,	nic395},
	{"tinker",	addr396,	0,	h_s12,	h_m9,	0},
	{"tycho",	addr397,	1,	h_s0,	h_m0,	nic397},
	{"ucb-arpa",	addr398,	1,	h_s0,	h_m2,	nic398},
	{"ucb-c70",	addr399,	1,	h_s0,	h_m8,	nic399},
	{"ucb-ing70",	addr400,	1,	h_s0,	h_m0,	nic400},
	{"ucb-ingvax",	addr401,	1,	h_s0,	h_m2,	nic401},
	{"ucla-ats",	addr402,	1,	h_s0,	h_m0,	nic402},
	{"ucla-ccn",	addr403,	1,	h_s9,	h_m31,	nic403},
	{"ucla-security",	addr404,	1,	h_s0,	h_m0,	nic404},
	{"udel-relay",	addr405,	1,	h_s0,	h_m2,	nic405},
	{"usafa-gateway",	addr406,	1,	h_s0,	h_m0,	nic406},
	{"usc-ecl",	addr407,	1,	h_s6,	h_m4,	nic407},
	{"usc-eclb",	addr408,	1,	h_s6,	h_m4,	nic408},
	{"usc-eclc",	addr409,	1,	h_s6,	h_m4,	nic409},
	{"usc-isi",	addr410,	1,	h_s15,	h_m4,	nic410},
	{"usc-isib",	addr411,	1,	h_s6,	h_m4,	nic411},
	{"usc-isic",	addr412,	1,	h_s6,	h_m4,	nic412},
	{"usc-isid",	addr413,	1,	h_s6,	h_m4,	nic413},
	{"usc-isie",	addr414,	1,	h_s6,	h_m4,	nic414},
	{"usc-isif",	addr415,	1,	h_s6,	h_m4,	nic415},
	{"usc-tac",	addr416,	0,	h_s4,	h_m1,	0},
	{"usgs1-amdahl",	addr417,	1,	h_s12,	h_m32,	nic417},
	{"usgs1-multics",	addr418,	1,	h_s21,	h_m15,	nic418},
	{"usgs1-tip",	addr419,	0,	h_s1,	h_m1,	0},
	{"usgs2-multics",	addr420,	1,	h_s21,	h_m15,	nic420},
	{"usgs2-tac",	addr421,	0,	h_s4,	h_m1,	0},
	{"usgs3-multics",	addr422,	1,	h_s21,	h_m15,	0},
	{"usgs3-tac",	addr423,	0,	h_s4,	h_m1,	0},
	{"utah-20",	addr424,	1,	h_s6,	h_m4,	nic424},
	{"utah-tip",	addr425,	0,	h_s1,	h_m1,	0},
	{"utexas-11",	addr426,	1,	h_s0,	h_m0,	nic426},
	{"utexas-20",	addr427,	1,	h_s6,	h_m4,	nic427},
	{"uwisc",	addr428,	1,	h_s0,	h_m2,	nic428},
	{"washington",	addr429,	1,	h_s6,	h_m4,	nic429},
	{"washington-tac",	addr430,	0,	h_s4,	h_m1,	0},
	{"wharton-10",	addr431,	1,	h_s19,	h_m4,	nic431},
	{"wpafb",	addr432,	1,	h_s7,	h_m3,	0},
	{"wpafb-afwal",	addr433,	1,	h_s19,	h_m4,	nic433},
	{"wpafb-tip",	addr434,	0,	h_s1,	h_m1,	0},
	{"wralc-tip",	addr435,	0,	h_s1,	h_m10,	0},
	{"wsmr-tip",	addr436,	0,	h_s1,	h_m1,	0},
	{"yale",	addr437,	1,	h_s0,	h_m0,	0},
	{"ypg",	addr438,	0,	h_s53,	h_m0,	0},
};
