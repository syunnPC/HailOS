#include "acpi.h"
#include "basetype.h"
#include "common.h"
#include "util.h"
#include "io.h"
#include "system_console.h"
#include "status.h"

static inline void MMIOWrite16Phys(addr_t Address, u16 Value)
{
    volatile u16* p = (volatile u16*)Address;
    *p = Value;
}

static inline bool CheckSigEq(const char* S1, const char* S2, size_t Length)
{
    for(size_t i=0; i<Length; i++)
    {
        if(S1[i] != S2[i])
        {
            return false;
        }
    }

    return true;
}

static bool ValidateRSDPChecksum(acpi_rsdp_t* Rsdp)
{
    if(Rsdp == NULL)
    {
        return false;
    }

    // ACPI v1 - 最初の20バイトの和が0
    if(Rsdp->Revision < 2)
    {
        u8* p = (u8*)Rsdp;
        u8 sum = 0;
        for(size_t i=0; i<20; i++)
        {
            sum = (u8)(sum + p[i]);
        }
        return sum == 0;
    }
    else //ACPI v2 - 最初の Rsdp->Length バイトの和が0
    {
        u8* p = (u8*)Rsdp;
        u32 len = Rsdp->Length;
        if(len < 20)
        {
            return false;
        }
        u8 sum = 0;
        for(u32 i=0; i<len; i++)
        {
            sum = (u8)(sum + p[i]);
        }
        return sum == 0;
    }
}

static acpi_rsdp_t* FindRSDP(void)
{
    u16 ebda_seg = *((u16*)(addr_t)0x040E);
    if(ebda_seg)
    {
        addr_t ebda_phys = (addr_t)ebda_seg << 4;
        addr_t start = ebda_phys;
        addr_t end = ebda_phys + 1024;
        for(addr_t addr = start; addr + sizeof(acpi_rsdp_t) <= end; addr += 16)
        {
            acpi_rsdp_t* rsdp = (acpi_rsdp_t*)(addr_t)addr;
            if(CheckSigEq(rsdp->Signature, RSDP_SIGNATURE, 8))
            {
                if(ValidateRSDPChecksum(rsdp))
                {
                    return rsdp;
                }
            }
        }
    }

    addr_t start = 0x000E0000;
    addr_t end = 0x000FFFFF;

    for(addr_t addr = start; addr + sizeof(acpi_rsdp_t) <= end; addr+= 16)
    {
        acpi_rsdp_t* rsdp = (acpi_rsdp_t*)(addr_t)addr;
        if(CheckSigEq(rsdp->Signature, RSDP_SIGNATURE, 8))
        {
            if(ValidateRSDPChecksum(rsdp))
            {
                return rsdp;
            }
        }
    }

    return NULL;
}

static acpi_sdt_header_t* FindTableXSDT(acpi_sdt_header_t* Xsdt, const char* Sign)
{
    if(Xsdt == NULL)
    {
        return NULL;
    }

    u32 len = Xsdt->Length;

    if(len < sizeof(acpi_sdt_header_t))
    {
        return NULL;
    }

    u32 entries = (len - sizeof(acpi_sdt_header_t)) / 8;
    u64* ptr = (u64*)((u8*)Xsdt + sizeof(acpi_sdt_header_t));

    for(u32 i=0; i<entries; i++)
    {
        acpi_sdt_header_t* h = (acpi_sdt_header_t*)(addr_t)ptr[i];
        if(!h)
        {
            continue;
        }
        if(CheckSigEq(h->Signature, Sign, 4))
        {
            return h;
        }
    }

    return NULL;
}

static acpi_sdt_header_t* FindTableRSDT(acpi_sdt_header_t* Rsdt, const char* Sign)
{
    if(Rsdt == NULL)
    {
        return NULL;
    }

    u32 len = Rsdt->Length;

    if(len < sizeof(acpi_sdt_header_t))
    {
        return NULL;
    }

    u32 entries = (len - sizeof(acpi_sdt_header_t)) / 4;
    u32* ptr = (u32*)((u8*)Rsdt + sizeof(acpi_sdt_header_t));

    for(u32 i=0; i<entries; i++)
    {
        acpi_sdt_header_t* h = (acpi_sdt_header_t*)(addr_t)ptr[i];
        if(!h)
        {
            continue;
        }
        if(CheckSigEq(h->Signature, Sign, 4))
        {
            return h;
        }
    }

    return NULL;
}

static acpi_fadt_min_t* FindFADTFromRSDP(acpi_rsdp_t* Rsdp)
{
    if(Rsdp == NULL)
    {
        return NULL;
    }

    if(Rsdp->Revision >= 2 && Rsdp->XsdtAddress)
    {
        acpi_sdt_header_t* xsdt = (acpi_sdt_header_t*)(addr_t)Rsdp->XsdtAddress;
        if(xsdt == NULL)
        {
            return NULL;
        }
        return (acpi_fadt_min_t*)FindTableXSDT(xsdt, "FACP");
    }
    else
    {
        acpi_sdt_header_t* rsdt = (acpi_sdt_header_t*)(addr_t)Rsdp->RsdtAddress;
        if(!rsdt)
        {
            return NULL;
        }
        return (acpi_fadt_min_t*)FindTableRSDT(rsdt, "FACP");
    }
}

static acpi_sdt_header_t* FindDSDTFromFADT(acpi_fadt_min_t* Fadt)
{
    if(Fadt == NULL)
    {
        return NULL;
    }

    if(Fadt->X_Dsdt)
    {
        return (acpi_sdt_header_t*)(addr_t)Fadt->X_Dsdt;
    }
    if(Fadt->Dsdt)
    {
        return (acpi_sdt_header_t*)(addr_t)Fadt->Dsdt;
    }

    return NULL;
}

static bool ExtractS5FromDSDT(acpi_sdt_header_t* Dsdt, u8* OutSlpA, u8* OutSlpB)
{
    if(Dsdt == NULL)
    {
        return false;
    }

    u8* base = (u8*)Dsdt;
    u32 len = Dsdt->Length;

    if(len < sizeof(acpi_sdt_header_t))
    {
        return false;
    }

    for(u32 i=0; i + 3 < len; i++)
    {
        if(base[i] == '_' && base[i+1] == 'S' && base[i+2] == '5')
        {
            u32 scan_end = i + 128;
            if(scan_end > len)
            {
                scan_end = len;
            }
            
            for(u32 j = i+3; j + 1 < scan_end; j++)
            {
                u8 b = base[j];
                if(b == 0x0A && j+1 < scan_end)
                {
                    u8 v1 = base[j+1];
                    for(u32 k=j+2; k+1 < scan_end; k++)
                    {
                        if(base[k] == 0x0A && k+1 < scan_end)
                        {
                            u8 v2 = base[k+1];
                            *OutSlpA = v1;
                            *OutSlpB = v2;
                            return true;
                        }
                    }
                }

                if(base[j] < 0x20 && base[j+1] < 0x20)
                {
                    *OutSlpA = base[j];
                    *OutSlpB = base[j+1];
                    return true;
                }

                if(b == 0x12)
                {
                    u32 inner_end = j + 64;
                    if(inner_end > scan_end) inner_end = scan_end;
                    for(u32 kk = j+1; kk + 1 < inner_end; kk++)
                    {
                        if(base[kk] == 0x0A && kk + 1 < inner_end)
                        {
                            u8 v1 = base[kk+1];
                            for(u32 kk2 = kk+2; kk2 + 1 < inner_end; kk2++)
                            {
                                if(base[kk2] == 0x0A && kk2 + 1 < inner_end)
                                {
                                    u8 v2 = base[kk2+1];
                                    *OutSlpA = v1;
                                    *OutSlpB = v2;
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return false;
}

static bool FADTGetPM1(acpi_fadt_min_t* Fadt, u64* Pm1aAddr, bool* Pm1aIsIo, u64* Pm1bAddr, bool* Pm1bIsIo)
{
    if(!Fadt)
    {
        return false;
    }

    if(Fadt->X_Pm1aCtrlBlock.Address != 0)
    {
        acpi_gas_t* g = &Fadt->X_Pm1aCtrlBlock;
        if(g->AddressSpaceID == 0)
        {
            *Pm1aAddr = (u64)(u32)g->Address;
            *Pm1aIsIo = true;
        }
        else
        {
            *Pm1aAddr = g->Address;
            *Pm1aIsIo = false;
        }
    }
    else if(Fadt->Pm1aCtrlBlock != 0)
    {
        *Pm1aAddr = (u64)Fadt->Pm1aCtrlBlock;
        *Pm1aIsIo = true;
    }
    else
    {
        return false;
    }

    if(Fadt->X_Pm1bCtrlBlock.Address != 0)
    {
        acpi_gas_t* g = &Fadt->X_Pm1bCtrlBlock;
        if(g->AddressSpaceID == 0)
        {
            *Pm1bAddr = (u64)(u32)g->Address;
            *Pm1bIsIo = true;
        }
        else
        {
            *Pm1bAddr = g->Address;
            *Pm1bIsIo = false;
        }
    }
    else if(Fadt->Pm1bCtrlBlock != 0)
    {
        *Pm1bAddr = (u64)Fadt->Pm1bCtrlBlock;
        *Pm1bIsIo = true;
    }
    else
    {
        *Pm1bAddr = 0;
        *Pm1bIsIo = false;
    }

    return true;
}

static void WritePM1Sleep(u64 Pm1aAddr, bool Pm1aIsIo, u64 Pm1bAddr, bool Pm1bIsIo, u16 SlpTypA, u16 SlpTypB)
{
    const u16 SLP_EN = 1<<13;
    const u16 SLP_TYP_SHIFT = 10;
    u16 val_a = (u16)((SlpTypA << SLP_TYP_SHIFT) | SLP_EN);

    asm volatile ("cli");

    if(Pm1aIsIo)
    {
        outw((u16)Pm1aAddr, val_a);
    }
    else
    {
        MMIOWrite16Phys(Pm1aAddr, val_a);
    }

    if(Pm1bAddr != 0)
    {
        u16 val_b = (u16)((SlpTypB << SLP_TYP_SHIFT) | SLP_EN);
        if(Pm1bIsIo)
        {
            outw((u16)Pm1bAddr, val_b);
        }
        else
        {
            MMIOWrite16Phys(Pm1bAddr, val_b);
        }
    }

    HaltProcessor();
}

bool ACPIPoweroffAutodetect(void)
{
    acpi_rsdp_t* rsdp = FindRSDP();
    if(rsdp == NULL)
    {
        PANIC(STATUS_ACPI_ERROR, 0);
    }

    acpi_fadt_min_t* fadt = FindFADTFromRSDP(rsdp);
    if(fadt == NULL)
    {
        PANIC(STATUS_ACPI_ERROR, 0);
    }

    acpi_sdt_header_t* dsdt = FindDSDTFromFADT(fadt);
    if(dsdt == NULL)
    {
        PANIC(STATUS_ACPI_ERROR, 0);
    }

    u8 slp_a = 0, slp_b = 0;
    bool found = ExtractS5FromDSDT(dsdt, &slp_a, &slp_b);
    if(!found)
    {
        slp_a = 0x05;
        slp_b = 0x05;
    }

    u64 pm1a_addr = 0, pm1b_addr = 0;
    bool pm1a_is_io = true, pm1b_is_io = false;

    if(!FADTGetPM1(fadt, &pm1a_addr, &pm1a_is_io, &pm1b_addr, &pm1b_is_io))
    {
        PANIC(STATUS_ACPI_ERROR, (int)found);
    }

    WritePM1Sleep(pm1a_addr, pm1a_is_io, pm1b_addr, pm1b_is_io, (u16)slp_a, (u16)slp_b);

    return true;
}

NORETURN void ACPIShutdown(void)
{
    if(!ACPIPoweroffAutodetect())
    {
        puts("ACPI Shutdown failed.\r\n");
        HaltProcessor();
    }
}