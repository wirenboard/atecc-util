#include "atecc_config_zone.h"
#include "atca_command.h"

void dump_slot_config(uint16_t value) {
    printf("Decoding SlotConfig value = 0x%04X\n", value);

    printf("Read key (except ECC private keys): %u\n", (value & SLOT_CONFIG_READ_KEY_MASK) >> SLOT_CONFIG_READ_KEY_OFFSET );
    printf(" If slot contains ECC private keys:\n");
    printf("  External signatures of arbitrary messages are enabled: %d\n", !!(value & SLOT_CONFIG_EXT_SIGN_ENABLED));
    printf("  Internal signatures are enabled: %d\n", !!(value & SLOT_CONFIG_INT_SIGN_ENABLED));
    printf("  ECDH operation is permitted for this key: %d\n", !!(value & SLOT_CONFIG_ECDH_PERMITTED));
    printf("   ECDH master secret output mode: %d\n", !!(value & SLOT_CONFIG_ECDH_MASTER_SECRET_MODE));
    

    printf("NoMac bit: %d\n", !!(value & SLOT_CONFIG_NO_MAC)  );
    printf("LimitedUse bit: %d\n", !!(value & SLOT_CONFIG_LIMITED_USE)  );
    printf("EncryptRead bit: %d\n", !!(value & SLOT_CONFIG_ENCRYPT_READ)  );
    printf("IsSecret bit: %d\n", !!(value & SLOT_CONFIG_IS_SECRET)  );
    printf("Write key: %u\n", (value & SLOT_CONFIG_WRITE_KEY_MASK) >> SLOT_CONFIG_WRITE_KEY_OFFSET );
    
    
    unsigned write_config =  (value & SLOT_CONFIG_WRITE_CONFIG_MASK) >> SLOT_CONFIG_WRITE_CONFIG_OFFSET ;

    printf("Write config: 0x%0X (hex) = %d%d%d%d (bin)\n", write_config,  
        !!(write_config & BIT(3)),
        !!(write_config & BIT(2)),
        !!(write_config & BIT(1)),
        !!(write_config & BIT(0))
    );

    // 'Write' configuration bits
    printf("  Write cmd: ");
    if (write_config == 0b0000) {
        printf("Always");
    } else if (write_config == 0b0001) {
        printf("PubInvalid");
    } else if ( (write_config & 0b1110) == 0b0010) {
        printf("Never");
    } else if ( (write_config & 0b1100) == 0b1000) {
        printf("Never");
    } else if ( (write_config & 0b0100) == 0b0100) {
        printf("Encrypt");
    } else {
        printf("Unknown");
    }
    printf("\n");

    // 'DeriveKey' configuration bits
    printf("  DeriveKey cmd: ");
    if ( (write_config & 0b1011) == 0b0010) {
        printf("Roll without MAC");
    } else if ( (write_config & 0b1011) == 0b1010) {
        printf("Roll with MAC");
    } else if ( (write_config & 0b1011) == 0b0011) {
        printf("Create without MAC");
    } else if ( (write_config & 0b1011) == 0b1011) {
        printf("Create with MAC");
    } else if ( (write_config & 0b0010) == 0b0000) {
        printf("Can't be used");
    } else {
        printf("Unknown");
    }
    printf("\n");

    // 'GenKey' configuration bits
    printf("  GenKey cmd: %s \n", (write_config & BIT(1)) ? "may be used" : "may NOT be used");
    // 'PrivWrite' configuration bits
    printf("  PrivWrite cmd: %s \n", (write_config & BIT(2)) ? "Encrypt" : "Forbidden");

}

void dump_key_config(uint16_t value) {
    printf("Decoding KeyConfig value = 0x%04X\n", value);

    printf("Private bit: %d\n", !!(value & KEY_CONFIG_PRIVATE)  );
    printf("PubInfo bit: %d\n", !!(value & KEY_CONFIG_PUB_INFO)  );

    uint8_t key_type = (value & KEY_CONFIG_KEY_TYPE_MASK) >> KEY_CONFIG_KEY_TYPE_OFFSET;
    printf("KeyType: %u [", key_type);
    switch (key_type) {
        case ATCA_B283_KEY_TYPE:
            printf("B283");
            break;
        case ATCA_K283_KEY_TYPE:
            printf("K283");
            break;
        case ATCA_P256_KEY_TYPE:
            printf("P256");
            break;
        case ATCA_AES_KEY_TYPE:
            printf("AES");
            break;
        case ATCA_SHA_KEY_TYPE:
            printf("SHA or other");
            break;
    }
    printf("]\n");

    printf("Lockable bit: %d\n", !!(value & KEY_CONFIG_LOCKABLE)  );
    printf("ReqRandom bit: %d\n", !!(value & KEY_CONFIG_REQ_RANDOM)  );
    printf("ReqAuth bit: %d\n", !!(value & KEY_CONFIG_REQ_AUTH)  );
    printf("AuthKey: %u\n", (value & KEY_CONFIG_AUTH_KEY_MASK) >> KEY_CONFIG_AUTH_KEY_OFFSET );
    printf("IntrusionDisable bit: %d\n", !!(value & KEY_CONFIG_INTRUSION_DISABLE)  );
    printf("X509id: %u\n", (value & KEY_CONFIG_X509_ID_MASK) >> KEY_CONFIG_X509_ID_OFFSET );
}

uint16_t swap_bytes_16(uint16_t val) {
    return ((val & 0x00FF) << 8) | (val & 0xFF00);
};

uint32_t parse_counter_value(uint8_t data[4])
{
    uint32_t value = 0;
    for (size_t i = 0; i < 4; ++i) {
        value <<= 8;
        value |= data[i];
    }
    return value;
}