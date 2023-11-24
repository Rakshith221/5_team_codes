/*
 * Copyright IBM Corp. All Rights Reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

'use strict';

// Deterministic JSON.stringify()
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
                BPM: 100,
                HRV: 10,
                },
                Time: '2023-11-04T15:30:00.000Z',
                Team: 'team3',
            },
        ];

        for (const asset of assets) {
            asset.docType = 'asset';
            // example of how to write to world state deterministically
            // use convetion of alphabetic order
            // we insert data in alphabetic order using 'json-stringify-deterministic' and 'sort-keys-recursive'
            // when retrieving data, in any lang, the order of data will be the same and consequently also the corresonding hash
            await ctx.stub.putState(asset.RID, Buffer.from(stringify(sortKeysRecursive(asset))));
        }
    }

    // CreateAsset issues a new asset to the world state with given details.
    async CreateAsset(ctx, rid, sid, bpm, hrv, time, team) {
        const exists = await this.AssetExists(ctx, rid);
        if (exists) {
            throw new Error(`The asset ${rid} already exists`);
        }

        const asset = {
            RID: rid,
            SID: sid,
            Readings: {
            BPM: bpm,
            HRV: hrv,
            },
            Time: time,
            Team: team,
        };
        // we insert data in alphabetic order using 'json-stringify-deterministic' and 'sort-keys-recursive'
        await ctx.stub.putState(rid, Buffer.from(stringify(sortKeysRecursive(asset))));
        return JSON.stringify(asset);
    }

    // ReadAsset returns the asset stored in the world state with given id.
    async ReadAsset(ctx, rid) {
        const assetJSON = await ctx.stub.getState(rid); // get the asset from chaincode state
        if (!assetJSON || assetJSON.length === 0) {
            throw new Error(`The asset ${rid} does not exist`);
        }
        return assetJSON.toString();
    }
   
   

    // UpdateAsset updates an existing asset in the world state with provided parameters.
    async UpdateAsset(ctx, rid, sid, bpm, hrv, time, team) {
        const exists = await this.AssetExists(ctx, rid);
        if (!exists) {
            throw new Error(`The asset ${rid} does not exist`);
        }

        // overwriting original asset with new asset
        const updatedAsset = {
            RID: rid,
            SID: sid,
            Readings: {
            BPM: bpm,
            HRV: hrv,
            },
            TIme: time,
            Team: team
        };
        // we insert data in alphabetic order using 'json-stringify-deterministic' and 'sort-keys-recursive'
        return ctx.stub.putState(rid, Buffer.from(stringify(sortKeysRecursive(updatedAsset))));
    }

    // DeleteAsset deletes an given asset from the world state.
    async DeleteAsset(ctx, rid) {
        const exists = await this.AssetExists(ctx, rid);
        if (!exists) {
            throw new Error(`The asset ${rid} does not exist`);
        }
        return ctx.stub.deleteState(rid);
    }

    // AssetExists returns true when asset with given ID exists in world state.
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
