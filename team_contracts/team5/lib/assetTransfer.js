/*
 * Copyright IBM Corp. All Rights Reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

'use strict';

const stringify  = require('json-stringify-deterministic');
const sortKeysRecursive  = require('sort-keys-recursive');
const { Contract } = require('fabric-contract-api');

class AssetTransfer extends Contract {

    async InitLedger(ctx) {
        const assets = [
            {
                RID: '101',
                SID: '1',
                Readings: {
                Latitude: 40,
                Longitude: 40,
                Power: 0,
                Equipment_ID: 60,
                RFID: 2
                },
                Time: '2023-11-04T15:30:00.000Z',
                Team: 'team5',
            },
        ];

        for (const asset of assets) {
            asset.docType = 'asset';
            await ctx.stub.putState(asset.RID, Buffer.from(stringify(sortKeysRecursive(asset))));
        }
    }

    async CreateAsset(ctx, rid, sid, lat, long, power, equipment_id, rfid, time, team) {
        const exists = await this.AssetExists(ctx, rid);
        if (exists) {
            throw new Error(`The asset ${rid} already exists`);
        }

        const asset = {
            RID: rid,
            SID: sid,
            Readings: {
            Latitude: lat,
            Longitude: long,
            Power: power,
            Equipment_ID: equipment_id,
            RFID: rfid
            },
            Time: time,
            Team: team,
        };
        await ctx.stub.putState(rid, Buffer.from(stringify(sortKeysRecursive(asset))));
        return JSON.stringify(asset);
    }

    async ReadAsset(ctx, rid) {
        const assetJSON = await ctx.stub.getState(rid);
        if (!assetJSON || assetJSON.length === 0) {
            throw new Error(`The asset ${rid} does not exist`);
        }
        return assetJSON.toString();
    }
   
   

    async UpdateAsset(ctx, rid, sid, lat, long, power, equipment_id, rfid, team) {
        const exists = await this.AssetExists(ctx, rid);
        if (!exists) {
            throw new Error(`The asset ${rid} does not exist`);
        }

        const updatedAsset = {
            RID: rid,
            SID: sid,
            Readings: {
            Latitude: lat,
            Longitude: long,
            Power: power,
            Equipment_ID: equipment_id,
            RFID: rfid
            },
            Time: time,
            Team: team,
        };
        return ctx.stub.putState(rid, Buffer.from(stringify(sortKeysRecursive(updatedAsset))));
    }

    async DeleteAsset(ctx, rid) {
        const exists = await this.AssetExists(ctx, rid);
        if (!exists) {
            throw new Error(`The asset ${rid} does not exist`);
        }
        return ctx.stub.deleteState(rid);
    }

    async AssetExists(ctx, rid) {
        const assetJSON = await ctx.stub.getState(rid);
        return assetJSON && assetJSON.length > 0;
    }
/*
    // TransferAsset updates the owner field of asset with given id in the world state.
    async TransferAsset(ctx, rid, newOwner) {
        const assetString = await this.ReadAsset(ctx, rid);
        const asset = JSON.parse(assetString);
        const oldOwner = asset.Owner;
        asset.Owner = newOwner;
        // we insert data in alphabetic order using 'json-stringify-deterministic' and 'sort-keys-recursive'
        await ctx.stub.putState(rid, Buffer.from(stringify(sortKeysRecursive(asset))));
        return oldOwner;
    }
*/
    // GetAllAssets returns all assets found in the world state.
    async GetAllAssets(ctx) {
        const allResults = [];
        // range query with empty string for startKey and endKey does an open-ended query of all assets in the chaincode namespace.
        const iterator = await ctx.stub.getStateByRange('', '');
        let result = await iterator.next();
        while (!result.done) {
            const strValue = Buffer.from(result.value.value.toString()).toString('utf8');
            let record;
            try {
                record = JSON.parse(strValue);
            } catch (err) {
                console.log(err);
                record = strValue;
            }
            allResults.push(record);
            result = await iterator.next();
        }
        return JSON.stringify(allResults);
    }
   
}

module.exports = AssetTransfer;


