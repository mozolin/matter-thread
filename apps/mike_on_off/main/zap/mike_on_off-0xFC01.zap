{
  "fileFormat": 2,
  "featureLevel": 107,
  "creator": "zap",
  "keyValuePairs": [
    {
      "key": "commandDiscovery",
      "value": "1"
    },
    {
      "key": "defaultResponsePolicy",
      "value": "always"
    },
    {
      "key": "manufacturerCodes",
      "value": "0x0000"
    }
  ],
  "package": [
    {
      "pathRelativity": "relativeToZap",
      "path": "../../../../connectedhomeip/connectedhomeip/src/app/zap-templates/zcl/zcl.json",
      "type": "zcl-properties",
      "category": "matter",
      "version": 1,
      "description": "Matter SDK ZCL data"
    },
    {
      "pathRelativity": "relativeToZap",
      "path": "../../../../connectedhomeip/connectedhomeip/src/app/zap-templates/app-templates.json",
      "type": "gen-templates-json",
      "category": "matter",
      "version": "chip-v1"
    },
    {
      "pathRelativity": "relativeToZap",
      "path": "mike_on_off-0xFC01-zapui.xml",
      "type": "zcl-xml-standalone"
    }
  ],
  "endpointTypes": [
    {
      "id": 1,
      "name": "MA-rootdevice",
      "deviceTypeRef": {
        "code": 22,
        "profileId": 259,
        "label": "MA-rootdevice",
        "name": "MA-rootdevice",
        "deviceTypeOrder": 0
      },
      "deviceTypes": [
        {
          "code": 22,
          "profileId": 259,
          "label": "MA-rootdevice",
          "name": "MA-rootdevice",
          "deviceTypeOrder": 0
        }
      ],
      "deviceVersions": [
        0
      ],
      "deviceIdentifiers": [
        22
      ],
      "deviceTypeName": "MA-rootdevice",
      "deviceTypeCode": 22,
      "deviceTypeProfileId": 259,
      "clusters": [
        {
          "name": "Identify",
          "code": 3,
          "mfgCode": null,
          "define": "IDENTIFY_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "Identify",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "IdentifyTime",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x0",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "IdentifyType",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "type": "IdentifyTypeEnum",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0x00",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "GeneratedCommandList",
              "code": 65528,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "AcceptedCommandList",
              "code": 65529,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "AttributeList",
              "code": 65531,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "FeatureMap",
              "code": 65532,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap32",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClusterRevision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "4",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Groups",
          "code": 4,
          "mfgCode": null,
          "define": "GROUPS_CLUSTER",
          "side": "server",
          "enabled": 1,
          "commands": [
            {
              "name": "AddGroupResponse",
              "code": 0,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "AddGroup",
              "code": 0,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "ViewGroupResponse",
              "code": 1,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "ViewGroup",
              "code": 1,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "GetGroupMembership",
              "code": 2,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "GetGroupMembershipResponse",
              "code": 2,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RemoveGroupResponse",
              "code": 3,
              "mfgCode": null,
              "source": "server",
              "isIncoming": 0,
              "isEnabled": 1
            },
            {
              "name": "RemoveGroup",
              "code": 3,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "RemoveAllGroups",
              "code": 4,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            },
            {
              "name": "AddGroupIfIdentifying",
              "code": 5,
              "mfgCode": null,
              "source": "client",
              "isIncoming": 1,
              "isEnabled": 1
            }
          ],
          "attributes": [
            {
              "name": "NameSupport",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "NameSupportBitmap",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "GeneratedCommandList",
              "code": 65528,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "AcceptedCommandList",
              "code": 65529,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "AttributeList",
              "code": 65531,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "FeatureMap",
              "code": 65532,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap32",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "0",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClusterRevision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "RAM",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": "4",
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        },
        {
          "name": "Descriptor",
          "code": 29,
          "mfgCode": null,
          "define": "DESCRIPTOR_CLUSTER",
          "side": "server",
          "enabled": 1,
          "attributes": [
            {
              "name": "DeviceTypeList",
              "code": 0,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ServerList",
              "code": 1,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClientList",
              "code": 2,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "PartsList",
              "code": 3,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "GeneratedCommandList",
              "code": 65528,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "AcceptedCommandList",
              "code": 65529,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "AttributeList",
              "code": 65531,
              "mfgCode": null,
              "side": "server",
              "type": "array",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "FeatureMap",
              "code": 65532,
              "mfgCode": null,
              "side": "server",
              "type": "bitmap32",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            },
            {
              "name": "ClusterRevision",
              "code": 65533,
              "mfgCode": null,
              "side": "server",
              "type": "int16u",
              "included": 1,
              "storageOption": "External",
              "singleton": 0,
              "bounded": 0,
              "defaultValue": null,
              "reportable": 1,
              "minInterval": 1,
              "maxInterval": 65534,
              "reportableChange": 0
            }
          ]
        }
      ]
    }
  ],
  "endpoints": [
    {
      "endpointTypeName": "MA-rootdevice",
      "endpointTypeIndex": 0,
      "profileId": 259,
      "endpointId": 9,
      "networkId": null,
      "parentEndpointIdentifier": null
    }
  ]
}