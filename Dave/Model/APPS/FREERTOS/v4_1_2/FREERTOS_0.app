<?xml version="1.0" encoding="ASCII"?>
<ResourceModel:App xmi:version="2.0" xmlns:xmi="http://www.omg.org/XMI" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ResourceModel="http://www.infineon.com/Davex/Resource.ecore" name="FREERTOS" URI="http://resources/4.1.2/app/FREERTOS/0" description="FreeRTOS is an open source real-time operating system (RTOS) for embedded systems" version="4.1.2" minDaveVersion="4.0.0" instanceLabel="FREERTOS_0" appLabel="">
  <properties singleton="true" sharable="true"/>
  <requiredApps URI="http://resources/4.1.2/app/FREERTOS/0/appres_clock" requiredAppName="CLOCK_XMC1" requiringMode="SHARABLE">
    <downwardMapList xsi:type="ResourceModel:App" href="../../CLOCK_XMC1/v4_0_20/CLOCK_XMC1_0.app#/"/>
  </requiredApps>
  <requiredApps URI="http://resources/4.1.2/app/FREERTOS/0/appres_cpu" requiredAppName="CPU_CTRL_XMC1" requiringMode="SHARABLE">
    <downwardMapList xsi:type="ResourceModel:App" href="../../CPU_CTRL_XMC1/v4_0_8/CPU_CTRL_XMC1_0.app#/"/>
  </requiredApps>
  <hwResources name="systick" URI="http://resources/4.1.2/app/FREERTOS/0/hwres_systick" resourceGroupUri="peripheral/cpu/0/systick" mResGrpUri="peripheral/cpu/0/systick">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/cpu/cpu_0.dd#//@provided.7"/>
  </hwResources>
  <hwResources name="excp_systick" URI="http://resources/4.1.2/app/FREERTOS/0/hwres_exception_systick" resourceGroupUri="peripheral/cpu/0/exception/systick" mResGrpUri="peripheral/cpu/0/exception/systick">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/cpu/cpu_0.dd#//@provided.6"/>
  </hwResources>
  <hwResources name="excp_pendsv" URI="http://resources/4.1.2/app/FREERTOS/0/hwres_exception_pendsv" resourceGroupUri="peripheral/cpu/0/exception/pendsv" mResGrpUri="peripheral/cpu/0/exception/pendsv">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/cpu/cpu_0.dd#//@provided.9"/>
  </hwResources>
  <hwResources name="excp_svcall" URI="http://resources/4.1.2/app/FREERTOS/0/hwres_exception_svcall" resourceGroupUri="peripheral/cpu/0/exception/svcall" mResGrpUri="peripheral/cpu/0/exception/svcall">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/cpu/cpu_0.dd#//@provided.8"/>
  </hwResources>
</ResourceModel:App>
