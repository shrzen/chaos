include $(PWD)/../Kbuild

all:
	make -C $(LINUX_DIR) M=$(PWD) src=$(PWD)/.. modules 

clean:
	make -C $(LINUX_DIR) M=$(PWD) src=$(PWD)/.. clean
